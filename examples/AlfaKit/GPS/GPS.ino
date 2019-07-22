#include <BreakoutSDK.h>
#include <Countdown.h>
// https://github.com/eclipse/paho.mqtt.embedded-c
#include <MQTTClient.h>
#include <massive-sdk/src/shims/paho-mqtt/RN4PahoIPStack.h>
#include <platform/ArduinoSeeedOwlSerial.h>
#include <stdio.h>
#include "config.h"

// BEGIN CONFIGURATION SECTION

#define LOOP_INTERVAL (1 * 1000)
#define SEND_INTERVAL (10 * 60 * 1000)

// END CONFIGURATION SECTION

#define MAX_MQTT_PACKET_SIZE 500

OwlModemRN4 *rn4_modem                               = nullptr;
ArduinoSeeedUSBOwlSerial *debug_serial               = nullptr;
ArduinoSeeedHwOwlSerial *modem_serial                = nullptr;
ArduinoSeeedHwOwlSerial *gnss_serial                 = nullptr;
RN4PahoIPStack *ip_stack                             = nullptr;
MQTT::Client<RN4PahoIPStack, Countdown, MAX_MQTT_PACKET_SIZE> *paho_client = nullptr;

str imei = { .s = nullptr, .len = 0 };

bool sleep = false;

static void device_state_callback(MQTT::MessageData &message) {
  str message_str = {.s = (char *)message.message.payload, .len = message.message.payloadlen};
  if (str_equal_char(message_str, "sleep")) {
    if (sleep) {
      LOG(L_INFO, "Already sleeping\r\n");
    } else {
      LOG(L_INFO, "Going to sleep\r\n");
      sleep = true;
    }
    return;
  }

  if (str_equal_char(message_str, "wakeup")) {
    if (!sleep) {
      LOG(L_INFO, "Already awake\r\n");
    } else {
      LOG(L_INFO, "Waking up\r\n");
      sleep = false;
    }
    return;
  }

  LOG(L_WARN, "Unknown state: %.*s\r\n", message_str.len, message_str.s);
}

void fail() {
  LOG(L_ERR, "Initialization failed, entering bypass mode. Reset the board to start again.\r\n");

  while (true) {
    if (SerialDebugPort.available()) {
      int bt = SerialDebugPort.read();
      SerialModule.write(bt);
    }

    if (SerialModule.available()) {
      int bt = SerialModule.read();
      SerialDebugPort.write(bt);
    }
  }
}

bool mqtt_connect() {
  if (!ip_stack->connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, USE_TLS, TLS_PROFILE_ID)) {
    LOG(L_WARN, "Failed to establish connection to the broker\r\n");
    return false;
  }

  paho_client                         = new MQTT::Client<RN4PahoIPStack, Countdown, MAX_MQTT_PACKET_SIZE>(*ip_stack);
  MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
  connect_data.MQTTVersion            = 4;
  connect_data.clientID.cstring       = MQTT_CLIENT_ID;
#ifdef MQTT_LOGIN
  connect_data.username.cstring       = MQTT_LOGIN;
#endif
#ifdef MQTT_PASSWORD
  connect_data.password.cstring       = MQTT_PASSWORD;
#endif

  if (MQTT_KEEP_ALIVE > 0) {
    connect_data.keepAliveInterval = MQTT_KEEP_ALIVE;
  }

  if (paho_client->connect(connect_data) != 0) {
    LOG(L_WARN, "Failed to connect to MQTT broker\r\n");
    return false;
  }

  if (paho_client->subscribe(MQTT_STATE_TOPIC, MQTT::QOS0, device_state_callback) != 0) {
    LOG(L_WARN, "Failed to subscribe to \"state\" topic\r\n");
    return false;
  }

  return true;
}

void mqtt_disconnect() {
  LOG(L_INFO, "Disconnecting mqtt client and ip stack\r\n");
  if (paho_client) {
    paho_client->disconnect();
    paho_client = nullptr;
  }

  if (ip_stack) {
    ip_stack->disconnect();
  }
}

#if USE_TLS
void configure_tls() {
  str ca = {.s = TLS_SERVER_CA, .len = sizeof(TLS_SERVER_CA) - 1};
  rn4_modem->ssl.setServerCA(ca);

  str cert = {.s = TLS_DEVICE_CERT, .len = sizeof(TLS_DEVICE_CERT) - 1};
  rn4_modem->ssl.setDeviceCert(cert);

  str key = {.s = TLS_DEVICE_PKEY, .len = sizeof(TLS_DEVICE_PKEY) - 1};
  rn4_modem->ssl.setDevicePkey(key);

  rn4_modem->ssl.initContext(TLS_PROFILE_ID, TLS_CIPHER_SUITE);
}
#endif


void setup() {
  // Feel free to change the log verbosity. E.g. from most critical to most verbose:
  //   - errors: L_ALERT, L_CRIT, L_ERR, L_ISSUE
  //   - warnings: L_WARN, L_NOTICE
  //   - information & debug: L_INFO, L_DB, L_DBG, L_MEM
  // When logging, the additional L_CLI level ensure that the output will always be visible, no matter the set level.
  owl_log_set_level(L_INFO);
  LOG(L_WARN, "Arduino setup() starting up\r\n");
  debug_serial = new ArduinoSeeedUSBOwlSerial(&SerialDebugPort);
  modem_serial = new ArduinoSeeedHwOwlSerial(&SerialModule, SerialModule_Baudrate);
  gnss_serial  = new ArduinoSeeedHwOwlSerial(&SerialGNSS, SerialGNSS_Baudrate);
  rn4_modem    = new OwlModemRN4(modem_serial, debug_serial, gnss_serial);

  LOG(L_WARN, "Powering on module...");
  if (!rn4_modem->powerOn()) {
    LOG(L_WARN, "... error powering on.\r\n");
    fail();
  }
  owlPowerOn(OWL_POWER_GNSS);
  LOG(L_WARN, "... done powering on.\r\n");

  LOG(L_WARN, "Initializing the module and registering on the network...");
  char *cops = nullptr;
#ifdef MOBILE_OPERATOR
  cops = MOBILE_OPERATOR;
#endif

  at_cops_format_e cops_format = AT_COPS__Format__Numeric;

#ifdef MOBILE_OPERATOR_FORMAT
  cops_format = MOBILE_OPERATOR_FORMAT;
#endif
  if (!rn4_modem->initModem(TESTING_VARIANT_INIT, TESTING_APN, cops, cops_format)) {
    LOG(L_WARN, "... error initializing.\r\n");
    fail();
  }
  LOG(L_WARN, "... done initializing.\r\n");

  str imei_temp = { .s = nullptr, .len = 0};
  rn4_modem->information.getIMEI(&imei_temp);
  str_dup(imei, imei_temp);

  LOG(L_WARN, "Waiting for network registration...");
  if (!rn4_modem->waitForNetworkRegistration("Telemetry", TESTING_VARIANT_REG)) {
    LOG(L_WARN, "... error registering on the network.\r\n");
    fail();
  }
  LOG(L_WARN, "... done waiting.\r\n");

#if USE_TLS
  configure_tls();
#endif

  ip_stack = new RN4PahoIPStack(&rn4_modem->socket);

  mqtt_connect();

  LOG(L_WARN, "Arduino loop() starting up\r\n");
 
  return;

  out_of_memory:
  LOG(L_ERR, "Arduino loop() it out of memory\r\n");
}

bool send_data(char *data) {
  MQTT::Message message;
  message.qos        = MQTT::QOS0;
  message.retained   = false;
  message.dup        = false;
  message.payload    = data;
  message.payloadlen = strlen(data);
  return (paho_client->publish(MQTT_PUBLISH_TOPIC, message) == 0);
}

void loop() {
  static unsigned long last_send = 0;

  if (!ip_stack->connected()) {
    if (mqtt_connect() != 1) {
      LOG(L_WARN, "Reconnection failed\r\n");
    }
    delay(LOOP_INTERVAL);
    return;
  }

  if (!sleep) {
    gnss_data_t data;

    if (rn4_modem->gnss.getGNSSData(&data) && data.valid &&
        ((last_send == 0) || (millis() - last_send >= SEND_INTERVAL))) {
      last_send = millis();

      if (data.valid) {
        char commandText[512];
        snprintf(commandText, 512, "{\"device\":\"%.*s\",\"latitude\":\"%d %7.5f %s\",\"longitude\":\"%d %7.5f %s\"}", 
                 imei.len, imei.s,
                 data.position.latitude_degrees, data.position.latitude_minutes, data.position.is_north ? "N" : "S", 
                 data.position.longitude_degrees, data.position.longitude_minutes, data.position.is_west ? "W" : "E");
        if (!send_data(commandText)) {
          LOG(L_WARN, "Error publishing message: (client connected status: %d)\r\n", paho_client->isConnected());
        }
      }
    }
  }

  rn4_modem->AT.spin();

  if ((paho_client != nullptr) && (paho_client->yield(LOOP_INTERVAL) != 0)) {
    LOG(L_WARN, "Yield returned error - likely disconnected now: %d\r\n", paho_client->isConnected());
    mqtt_disconnect();
  }
}
