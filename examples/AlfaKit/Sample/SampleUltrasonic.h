#define SAMPLE_ULTRASONIC

// Install https://github.com/Seeed-Studio/Grove_Ultrasonic_Ranger
#include <Ultrasonic.h>

// BEGIN CONFIGURATION SECTION

// Use D38 Grove port
#define ULTRASONIC_PIN 38

// END CONFIGURATION SECTION

Ultrasonic UltrasonicRanger(ULTRASONIC_PIN);

void sample_ultrasonic_setup() {
  pinMode(ULTRASONIC_PIN, INPUT);
}

void sample_ultrasonic_loop() {
  if (!sleep) {
    long distance;
    distance             = UltrasonicRanger.MeasureInCentimeters();
    static bool detected = false;
    bool old_detected    = detected;
    if (!detected && distance < 10) {
      detected = true;
    } else if (detected && distance > 15) {
      detected = false;
    }
    if (old_detected != detected) {
      char commandText[512];
      snprintf(commandText, 512, "{\"device\":\"%.*s\",\"object\":\"%s\"}", imei.len, imei.s,
               (detected ? "detected" : "lost"));
      if (!send_data(commandText)) {
        LOG(L_WARN, "Error publishing message: (client connected status: %d)\r\n", mqtt->isConnected());
      }
    }
  }
}
