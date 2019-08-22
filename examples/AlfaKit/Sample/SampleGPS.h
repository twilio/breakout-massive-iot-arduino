// BEGIN CONFIGURATION SECTION

#define SAMPLE_GPS

#define GPS_SEND_INTERVAL (10 * 60 * 1000)

void sample_gps_setup() {
  owlPowerOn(OWL_POWER_GNSS);
}

void sample_gps_loop() {
  static unsigned long last_send = 0;

  if (!sleep) {
    gnss_data_t data;

    if (rn4_modem->gnss.getGNSSData(&data) && data.valid &&
        ((last_send == 0) || (millis() - last_send >= GPS_SEND_INTERVAL))) {
      if (data.valid) {
        last_send = millis();

        char commandText[512];
        snprintf(commandText, 512, "{\"device\":\"%.*s\",\"latitude\":\"%d %7.5f %s\",\"longitude\":\"%d %7.5f %s\"}",
                 imei.len, imei.s, data.position.latitude_degrees, data.position.latitude_minutes,
                 data.position.is_north ? "N" : "S", data.position.longitude_degrees, data.position.longitude_minutes,
                 data.position.is_west ? "W" : "E");
        if (!send_data(commandText)) {
          LOG(L_WARN, "Error publishing message: (client connected status: %d)\r\n", mqtt->isConnected());
        }
      }
    }
  }
}
