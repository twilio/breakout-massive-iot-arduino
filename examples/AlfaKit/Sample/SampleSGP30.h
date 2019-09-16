#define SAMPLE_SGP30

#include <sensirion_common.h>
#include <sgp30.h>

#define CO2_SEND_INTERVAL (20 * 1000)

void sample_sgp30_setup() {
  int16_t err;
  uint16_t scaled_ethanol_signal;
  uint16_t scaled_h2_signal;

  while (sgp_probe() != STATUS_OK) {
    LOG(L_ERR, "SGP failed");
    while(1);
  }
  err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal,
                                            &scaled_h2_signal);
  if (err == STATUS_OK) {
    LOG(L_INFO, "get ram signal!");
  } else {
    LOG(L_ERR, "error reading signals");
  }
  err = sgp_iaq_init();
}

void sample_sgp30_loop() {
  static unsigned long last_send = 0;

  if (!sleep && ((last_send == 0) || (millis() - last_send >= CO2_SEND_INTERVAL))) {
    int16_t err=0;
    uint16_t tvoc_ppb;
    uint16_t co2_eq_ppm;
    err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
    if (err == STATUS_OK) {
      last_send = millis();
      char commandText[512];
      snprintf(commandText, 512, "{\"device\":\"%.*s\",\"tVOC Concentration (ppb)\":%d,\"CO2eq Concentration (ppm)\":%d}", imei.len, imei.s, (int) tvoc_ppb, (int) co2_eq_ppm);
      if (!send_data(commandText)) {
        LOG(L_WARN, "Error publishing message: (client connected status: %d)\r\n", mqtt->isConnected());
      }
    } else {
      LOG(L_ERR, "error reading IAQ values\n");
    }
  }
}
