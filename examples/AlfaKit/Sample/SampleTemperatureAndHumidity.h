#define SAMPLE_TEMPERATURE_HUMIDITY

// https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor
#include <DHT.h>

// BEGIN CONFIGURATION SECTION

// Use D38 Grove port
#define SENSOR_PIN (D38)
#define DHTTYPE DHT11  // DHT 11

#define TEMP_SEND_INTERVAL (20 * 1000)

// END CONFIGURATION SECTION

DHT dht(SENSOR_PIN, DHTTYPE);

void sample_temperature_humidity_setup() {
  pinMode(SENSOR_PIN, INPUT);
}

void sample_temperature_humidity_loop() {
  static unsigned long last_send = 0;

  if (!sleep && ((last_send == 0) || (millis() - last_send >= TEMP_SEND_INTERVAL))) {
    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();

    last_send = millis();

    char commandText[512];
    snprintf(commandText, 512, "{\"device\":\"%.*s\",\"humidity\":%4.2f,\"temp\":%4.2f}", imei.len, imei.s, humidity,
             temperature);
    if (!send_data(commandText)) {
      LOG(L_WARN, "Error publishing message: (client connected status: %d)\r\n", mqtt->isConnected());
    }
  }
}
