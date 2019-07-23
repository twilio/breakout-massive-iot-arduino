// The Arduino Countdown timer library
#include <Countdown.h>

// The Eclipse Paho MQTT client
// https://github.com/eclipse/paho.mqtt.embedded-c
#include <MQTTClient.h>

// A Paho IP Stack implementation compatible with BreakoutSDK
#include <massive-sdk/src/shims/paho-mqtt/RN4PahoIPStack.h>

// Recommended buffer size defaults
#define MAX_MQTT_PACKET_SIZE 500

RN4PahoIPStack *ip_stack                             = nullptr;
MQTT::Client<RN4PahoIPStack, Countdown, MAX_MQTT_PACKET_SIZE> *paho_client = nullptr;

bool sleep = false;

// forward declarations
bool mqtt_connect();
static void device_state_callback(MQTT::MessageData &message);

void mqtt_setup() {
  ip_stack = new RN4PahoIPStack(&rn4_modem->socket);

  mqtt_connect();
}

bool mqtt_loop() {
  if (!ip_stack->connected()) {
    if (mqtt_connect() != 1) {
      LOG(L_WARN, "Reconnection failed\r\n");
    }
    delay(LOOP_INTERVAL);
    return false;
  }
  return true;
}

bool mqtt_connect() {
  bool use_tls;
#ifdef USE_TLS
  if (!ip_stack->connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, true, TLS_PROFILE_ID)) {
#else
  if (!ip_stack->connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, false)) {
#endif
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

bool send_data(char *data) {
  MQTT::Message message;
  message.qos        = MQTT::QOS0;
  message.retained   = false;
  message.dup        = false;
  message.payload    = data;
  message.payloadlen = strlen(data);
  return (paho_client->publish(MQTT_PUBLISH_TOPIC, message) == 0);
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
