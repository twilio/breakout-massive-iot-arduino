#include <BreakoutSDK.h>
#include <platform/ArduinoSeeedOwlSerial.h>
#include <stdio.h>
#include "config.h"

// https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor
#include <DHT.h>


OwlModemBG96 *bg96_modem              = nullptr;
ArduinoSeeedHwOwlSerial *grove_serial = nullptr;

#define FORCE_OVERRIDE_CREDENTIALS false
#define LOOP_INTERVAL (200)
#define SEND_INTERVAL (5 * 1000)

#define SENSOR_PIN (D38)
#define DHTTYPE DHT11  // DHT 11

DHT dht(SENSOR_PIN, DHTTYPE);

bool sleep = false;

static void device_state_callback(str topic, str data) {
  if (!str_equal_char(topic, MQTT_STATE_TOPIC)) {
    LOG(L_WARN, "Unknown topic: %.*s\r\n", topic.len, topic.s);
    LOG(L_WARN, "Unknown topic len: %d\r\n", topic.len);
    LOG(L_WARN, "Unknown topic should be: %s\r\n", MQTT_STATE_TOPIC);
    LOG(L_WARN, "Unknown topic should be len: %d\r\n", strlen(MQTT_STATE_TOPIC));
    return;
  }

  if (str_equal_char(data, "sleep")) {
    if (sleep) {
      LOG(L_INFO, "Already sleeping\r\n");
    } else {
      LOG(L_INFO, "Going to sleep\r\n");
      sleep = true;
    }
    return;
  }

  if (str_equal_char(data, "wakeup")) {
    if (!sleep) {
      LOG(L_INFO, "Already awake\r\n");
    } else {
      LOG(L_INFO, "Waking up\r\n");
      sleep = false;
    }
    return;
  }

  LOG(L_WARN, "Unknown state: %.*s\r\n", data.len, data.s);
}

void fail() {
  LOG(L_ERR, "Initialization failed, entering bypass mode. Reset the board to start again.\r\n");

  while (true) {
    if (SerialDebugPort.available()) {
      int bt = SerialDebugPort.read();
      SerialGrove.write(bt);
    }

    if (SerialGrove.available()) {
      int bt = SerialGrove.read();
      SerialDebugPort.write(bt);
    }
  }
}

void setup() {
  // Feel free to change the log verbosity. E.g. from most critical to most verbose:
  //   - errors: L_ALERT, L_CRIT, L_ERR, L_ISSUE
  //   - warnings: L_WARN, L_NOTICE
  //   - information & debug: L_INFO, L_DB, L_DBG, L_MEM
  // When logging, the additional L_CLI level ensure that the output will always be visible, no matter the set level.
  owl_log_set_level(L_INFO);
  LOG(L_WARN, "Arduino setup() starting up\r\n");

  grove_serial = new ArduinoSeeedHwOwlSerial(&SerialGrove, BG96_Baudrate);
  bg96_modem   = new OwlModemBG96(grove_serial);

  LOG(L_WARN, "Powering on module...");
  if (!bg96_modem->powerOn()) {
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
  if (!bg96_modem->initModem(TESTING_APN, cops, cops_format)) {
    LOG(L_WARN, "... error initializing.\r\n");
    fail();
  }
  LOG(L_WARN, "... done initializing.\r\n");

  LOG(L_WARN, "Waiting for network registration...");
  if (!bg96_modem->waitForNetworkRegistration(30000)) {
    LOG(L_WARN, "... error registering on the network.\r\n");
    fail();
  }
  LOG(L_WARN, "... done waiting.\r\n");

#if USE_TLS
  if (!bg96_modem->ssl.initContext()) {
    LOG(L_WARN, "Failed to initialize TLS context\r\n");
    fail();
  }

  if (!bg96_modem->ssl.setDeviceCert(STRDECL(TLS_DEVICE_CERT), FORCE_OVERRIDE_CREDENTIALS)) {
    LOG(L_WARN, "Failed to initialize TLS certificate\r\n");
    fail();
  }

  if (!bg96_modem->ssl.setDevicePkey(STRDECL(TLS_DEVICE_PKEY), FORCE_OVERRIDE_CREDENTIALS)) {
    LOG(L_WARN, "Failed to initialize TLS private key\r\n");
    fail();
  }

  if (!bg96_modem->ssl.setServerCA(STRDECL(TLS_SERVER_CA), FORCE_OVERRIDE_CREDENTIALS)) {
    LOG(L_WARN, "Failed to initialize TLS peer CA\r\n");
    fail();
  }
#endif

  bg96_modem->mqtt.useTLS(USE_TLS);

  if (!bg96_modem->mqtt.openConnection(MQTT_BROKER_HOST, MQTT_BROKER_PORT)) {
    LOG(L_WARN, "Failed to open connection to MQTT broker\r\n");
    fail();
  }

  if (!bg96_modem->mqtt.login(MQTT_CLIENT_ID, MQTT_LOGIN, MQTT_PASSWORD)) {
    LOG(L_WARN, "Failed to login to MQTT broker\r\n");
    fail();
  }

  bg96_modem->mqtt.setMessageCallback(device_state_callback);
  if (!bg96_modem->mqtt.subscribe("state", 1)) {
    LOG(L_WARN, "Failed to subscribe to \"state\" topic\r\n");
    fail();
  }

  LOG(L_WARN, "Arduino loop() starting up\r\n");
}

void loop() {
  static unsigned long last_send = 0;

  if (!sleep && ((last_send == 0) || (millis() - last_send >= SEND_INTERVAL))) {
    last_send = millis();

    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();

    char commandText[512];
    snprintf(commandText, 512, "Current humidity: [%4.2f] and current temp [%4.2f]", humidity, temperature);
    str commandStr = {commandText, strlen(commandText)};
    bg96_modem->mqtt.publish(MQTT_PUBLISH_TOPIC, commandStr);
  }

  bg96_modem->AT.spin();

  delay(LOOP_INTERVAL);
}
