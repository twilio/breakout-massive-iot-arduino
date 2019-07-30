#define SAMPLE_BUTTON

// BEGIN CONFIGURATION SECTION

// Use D38 Grove port
#define BUTTON_PIN 38

// END CONFIGURATION SECTION

void sample_button_setup() {
  pinMode(BUTTON_PIN, INPUT);
}

void sample_button_loop() {
  static bool pressed = true;

  if (!sleep) {
    int buttonState = digitalRead(BUTTON_PIN);
    if (!pressed && buttonState) {
      pressed = true;

      char commandText[512];
      snprintf(commandText, 512, "{\"device\":\"%.*s\",\"button\":\"pressed\"}", imei.len, imei.s);
      if (!send_data(commandText)) {
        LOG(L_WARN, "Error publishing message: (client connected status: %d)\r\n", mqtt->isConnected());
      }
    } else if (pressed && !buttonState) {
      LOG(L_INFO, "Button is not currently pressed\r\n");
      pressed = false;
    }
  }
}
