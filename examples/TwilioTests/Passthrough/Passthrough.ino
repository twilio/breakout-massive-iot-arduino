#include <BreakoutSDK.h>
#include <platform/ArduinoSeeedOwlSerial.h>

OwlModemRN4 *rn4_modem                 = nullptr;
ArduinoSeeedUSBOwlSerial *debug_serial = nullptr;
ArduinoSeeedHwOwlSerial *modem_serial  = nullptr;

void passthrough() {
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

void setup() {
  owl_log_set_level(L_INFO);

  debug_serial = new ArduinoSeeedUSBOwlSerial(&SerialDebugPort);
  modem_serial = new ArduinoSeeedHwOwlSerial(&SerialModule, SerialModule_Baudrate);
  rn4_modem    = new OwlModemRN4(modem_serial, debug_serial);

  LOG(L_WARN, "Powering on module...");
  if (!rn4_modem->powerOn()) {
    LOG(L_WARN, "... error powering on.\r\n");
    passthrough();
  }
  LOG(L_WARN, "... done powering on, modem is ready to respond to your commands\r\n");
  passthrough();
}

void loop() {
}
