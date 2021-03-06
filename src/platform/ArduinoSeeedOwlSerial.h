#include "massive-sdk/src/modem/IOwlSerial.h"
#include <usb_serial.h>
#include <HardwareSerial.h>

class ArduinoSeeedUSBOwlSerial : public IOwlSerial {
 public:
  ArduinoSeeedUSBOwlSerial(USBSerial *serial) : serial_(serial) {
    if (serial_ != nullptr) serial_->enableSmartBlockingTx(3*1000);
  }
  virtual ~ArduinoSeeedUSBOwlSerial() {
    if (serial_) serial_->end();
  }
  int32_t read(uint8_t *buf, uint32_t count) override {
    return (serial_ != nullptr) ? serial_->readBytes(buf, count) : -1;
  }
  int32_t available() override {
    return (serial_ != nullptr) ? serial_->available() : -1;
  }
  int32_t write(const uint8_t *buf, uint32_t count) override {
    return (serial_ != nullptr) ? serial_->write(buf, count) : -1;
  }

 private:
  USBSerial *serial_{nullptr};
};

class ArduinoSeeedHwOwlSerial : public IOwlSerial {
 public:
  ArduinoSeeedHwOwlSerial(HardwareSerial *serial, int baudrate) : serial_(serial) {
    serial_->begin(baudrate);
  }
  virtual ~ArduinoSeeedHwOwlSerial() {
    if (serial_) serial_->end();
  }
  int32_t read(uint8_t *buf, uint32_t count) override {
    return (serial_ != nullptr) ? serial_->readBytes(buf, count) : -1;
  }
  int32_t available() override {
    return (serial_ != nullptr) ? serial_->available() : -1;
  }
  int32_t write(const uint8_t *buf, uint32_t count) override {
    return (serial_ != nullptr) ? serial_->write(buf, count) : -1;
  }

 private:
  HardwareSerial *serial_{nullptr};
};
