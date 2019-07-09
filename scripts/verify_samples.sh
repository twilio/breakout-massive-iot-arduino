#!/bin/bash

# Initial setup
# compile and install https://github.com/eclipse/paho.mqtt.embedded-c/archive/master.zip
# arduino --install-library "Grove Ultrasonic Ranger"

board=Seeeduino:Seeed_STM32F4:wioGpsM4

# Verifying samples
for x in $(find examples/AlfaKit -name \*ino); do \
  echo "Verifying ${x}..."
  arduino --verify --board ${board} ${x} || break
done

for x in $(find examples/BG96 -name \*ino); do \
  echo "Verifying ${x}..."
  arduino --verify --board ${board} ${x} || break
done
