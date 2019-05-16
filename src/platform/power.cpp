#include "massive-sdk/src/platform/power.h"
#include "board.h"

void owlPowerOn(uint32_t power_bitmask) {
  if (power_bitmask & OWL_POWER_RN4) {
    // Set RTS pin down to enable UART communication
    pinMode(RTS_PIN, OUTPUT);
    digitalWrite(RTS_PIN, LOW);

    pinMode(MODULE_PWR_PIN, OUTPUT);
    digitalWrite(MODULE_PWR_PIN, HIGH);  // Module Power Default HIGH

    /* Is this just for the modem? or also the rest? */
    pinMode(PWR_KEY_PIN, OUTPUT);
    digitalWrite(PWR_KEY_PIN, LOW);
    digitalWrite(PWR_KEY_PIN, HIGH);
    delay(800);
    digitalWrite(PWR_KEY_PIN, LOW);
  }

  if (power_bitmask & OWL_POWER_BG96) {
    pinMode(GROVE_PWR_PIN, OUTPUT);
    digitalWrite(GROVE_PWR_PIN, HIGH);  // VCC_B Enable pin

    pinMode(BG96_RESET_PIN, OUTPUT);
    digitalWrite(BG96_RESET_PIN, LOW);

    delay(300);

    digitalWrite(BG96_RESET_PIN, HIGH);
  }

  if (power_bitmask & OWL_POWER_GNSS) {
    pinMode(GNSS_PWR_PIN, OUTPUT);
    digitalWrite(GNSS_PWR_PIN, HIGH);
  }
}

void owlPowerOff(uint32_t power_bitmask) {
  if (power_bitmask & OWL_POWER_RN4) {
    pinMode(MODULE_PWR_PIN, OUTPUT);
    digitalWrite(MODULE_PWR_PIN, HIGH);  // Module Power Default HIGH
  }

  if (power_bitmask & OWL_POWER_GNSS) {
    pinMode(GNSS_PWR_PIN, OUTPUT);
    digitalWrite(GNSS_PWR_PIN, LOW);
  }

  if (power_bitmask & OWL_POWER_BG96) {
    pinMode(BG96_RESET_PIN, OUTPUT);
    digitalWrite(BG96_RESET_PIN, LOW);
  }
}

