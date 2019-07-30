// BEGIN CONFIGURATION SECTION

// User configurable options
#include "config.h"

// END CONFIGURATION SECTION

#include "modem.h"

#include "mqtt.h"

// BEGIN SAMPLE SELECTION SECTION

// These three samples use the same port on the development board, D38, so only one can be used
// at a time by default.  You can edit the header file to specify D20 to use two of the
// components simultaneously.
//#include "SampleButton.h"
#include "SampleTemperatureAndHumidity.h"
//#include "SampleUltrasonic.h"

// This sample is independent of the grove input ports so can be used with any of the above
// samples.  Please keep in mind GPS reception is best either outside or very close to an
// outside facing window.
//#include "SampleGPS.h"

// END SAMPLE SELECTION SECTION

/**
 * Setting up the Arduino platform. This is executed once, at reset.
 */
void setup() {
  owl_log_set_level(L_INFO);

  modem_setup();
  mqtt_setup();

  LOG(L_WARN, "Arduino loop() starting up\r\n");

#ifdef SAMPLE_BUTTON
  sample_button_setup();
#endif
#ifdef SAMPLE_GPS
  sample_gps_setup();
#endif
#ifdef SAMPLE_TEMPERATURE_HUMIDITY
  sample_temperature_humidity_setup();
#endif
#ifdef SAMPLE_ULTRASONIC
  sample_ultrasonic_setup();
#endif

  return;
}

void loop() {
  static bool pressed = true;

  if (!mqtt_loop()) {
    goto continue_loop;
  }

#ifdef SAMPLE_BUTTON
  sample_button_loop();
#endif
#ifdef SAMPLE_GPS
  sample_gps_loop();
#endif
#ifdef SAMPLE_TEMPERATURE_HUMIDITY
  sample_temperature_humidity_loop();
#endif
#ifdef SAMPLE_ULTRASONIC
  sample_ultrasonic_loop();
#endif

continue_loop:
  rn4_modem->AT.spin();
  if (mqtt != nullptr) {
    mqtt->yield(LOOP_INTERVAL);
  }
}
