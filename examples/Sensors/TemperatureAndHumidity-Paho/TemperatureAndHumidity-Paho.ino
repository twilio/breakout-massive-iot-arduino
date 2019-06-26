#include <BreakoutSDK.h>
#include <Countdown.h>
// https://github.com/eclipse/paho.mqtt.embedded-c
#include <MQTTClient.h>
#include <massive-sdk/src/shims/paho-mqtt/RN4PahoIPStack.h>
#include <platform/ArduinoSeeedOwlSerial.h>
#include <stdio.h>

// https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor
#include <DHT.h>

#include "config.h"

// BEGIN CONFIGURATION SECTION

#define LOOP_INTERVAL (200)
#define SEND_INTERVAL (20 * 1000)

#define SENSOR_PIN (D38)
#define DHTTYPE DHT11  // DHT 11

// END CONFIGURATION SECTION

OwlModemRN4 *rn4_modem                               = nullptr;
ArduinoSeeedUSBOwlSerial *debug_serial               = nullptr;
ArduinoSeeedHwOwlSerial *modem_serial                = nullptr;
RN4PahoIPStack *ip_stack                             = nullptr;
MQTT::Client<RN4PahoIPStack, Countdown> *paho_client = nullptr;

DHT dht(SENSOR_PIN, DHTTYPE);

bool sleep = false;

static void device_state_callback(MQTT::MessageData &message) {
  if (strcmp((char *)message.message.payload, "sleep") == 0) {
    if (sleep) {
      LOG(L_INFO, "Already sleeping\r\n");
    } else {
      LOG(L_INFO, "Going to sleep\r\n");
      sleep = true;
    }
    return;
  }

  if (strcmp((char *)message.message.payload, "wakeup")) {
    if (!sleep) {
      LOG(L_INFO, "Already awake\r\n");
    } else {
      LOG(L_INFO, "Waking up\r\n");
      sleep = false;
    }
    return;
  }

  LOG(L_WARN, "Unknown state: %s\r\n", message.message.payload);
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

  paho_client                         = new MQTT::Client<RN4PahoIPStack, Countdown>(*ip_stack);
  MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
  connect_data.MQTTVersion            = 4;
  connect_data.clientID.cstring       = MQTT_CLIENT_ID;
  if (MQTT_KEEP_ALIVE > 0) {
    connect_data.keepAliveInterval    = MQTT_KEEP_ALIVE;
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
  str ca = {.s = TLS_SERVER_CA, .len = sizeof(TLS_SERVER_CA) -1};
  rn4_modem->ssl.setServerCA(ca);

  str cert = {.s = TLS_DEVICE_CERT, .len = sizeof(TLS_DEVICE_CERT) -1};
  rn4_modem->ssl.setDeviceCert(cert);

  str key = {.s = TLS_DEVICE_PKEY, .len = sizeof(TLS_DEVICE_PKEY) -1};
  rn4_modem->ssl.setDevicePkey(key);

  rn4_modem->ssl.initContext(TLS_PROFILE_ID, TLS_CIPHER_SUITE);
}
#endif

void setup() {
  owl_log_set_level(L_INFO);
  LOG(L_WARN, "Arduino setup() starting up\r\n");

  debug_serial = new ArduinoSeeedUSBOwlSerial(&SerialDebugPort);
  modem_serial = new ArduinoSeeedHwOwlSerial(&SerialModule, SerialModule_Baudrate);
  rn4_modem    = new OwlModemRN4(modem_serial, debug_serial);

  LOG(L_WARN, "Powering on module...");
  if (!rn4_modem->powerOn()) {
    LOG(L_WARN, "... error powering on.\r\n");
    fail();
  }
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
}

void loop() {
  static unsigned long last_send = 0;

  if (!sleep && ((last_send == 0) || (millis() - last_send >= SEND_INTERVAL))) {
    if (!ip_stack->connected()) {
      if (mqtt_connect() != 1) {
        LOG(L_WARN, "Connection failed\r\n");
      }
      return;
    }
    last_send = millis();

    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();

    char commandText[512];
    snprintf(commandText, 512, "Current humidity: [%4.2f] and current temp [%4.2f]", humidity, temperature);
    MQTT::Message message;
    message.qos        = MQTT::QOS0;
    message.retained   = false;
    message.dup        = false;
    message.payload    = commandText;
    message.payloadlen = strlen(commandText);
    int rc = paho_client->publish(MQTT_TOPIC, message);
    if (rc != 0) {
      LOG(L_WARN, "Error publishing message: %d (client connected: %d)\r\n", rc, paho_client->isConnected());
    }
  }

  rn4_modem->AT.spin();

  if ((paho_client != nullptr) && (paho_client->yield(LOOP_INTERVAL) != 0)) {
      LOG(L_WARN, "Yield returned error - likely disconnected now: %d\r\n", paho_client->isConnected());
      mqtt_disconnect();
  }
}
