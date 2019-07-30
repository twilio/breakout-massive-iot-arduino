#!/bin/bash

SOURCE_DIR=$(pwd)

set -e

# install Arduino IDE
mkdir ~/arduino
cd ~/arduino
wget https://downloads.arduino.cc/arduino-1.8.9-linux64.tar.xz
tar xf arduino-1.8.9-linux64.tar.xz
cd arduino-1.8.9
./install.sh
export PATH=${PATH}:$(pwd)

mkdir -p ~/.arduino15

# install Arduino CLI
mkdir ~/arduino-cli
cd ~/arduino-cli
wget https://github.com/arduino/arduino-cli/releases/download/0.3.7-alpha.preview/arduino-cli-0.3.7-alpha.preview-linux64.tar.bz2
tar xf arduino-cli-0.3.7-alpha.preview-linux64.tar.bz2
mv arduino-cli-0.3.7-alpha.preview-linux64 arduino-cli

# install core
cat <<EOF >arduino-cli.yaml
board_manager:
  additional_urls:
    - https://raw.githubusercontent.com/Seeed-Studio/Seeed_Platform/master/package_seeeduino_boards_index.json
EOF
./arduino-cli --config-file arduino-cli.yaml core update-index
./arduino-cli --config-file arduino-cli.yaml core install Seeeduino:Seeed_STM32F4

# install libraries from the registry
./arduino-cli --config-file arduino-cli.yaml lib update-index
./arduino-cli --config-file arduino-cli.yaml lib install 'Grove Temperature And Humidity Sensor'
./arduino-cli --config-file arduino-cli.yaml lib install 'Grove Ultrasonic Ranger'
./arduino-cli --config-file arduino-cli.yaml lib install 'ArduinoMqtt'

rm -f ~/.arduino15/package_index.json # arduino doesn't trust package_index.json from arduino-cli and crashes, so just delete it

# install libraries from Github
git clone https://github.com/eclipse/paho.mqtt.embedded-c.git
cd paho.mqtt.embedded-c
make arduino
cp -r ./build/output/arduino/MQTTClient ~/Arduino/libraries

# install Massive SDK
cp -r ${SOURCE_DIR} ~/Arduino/libraries

# Magic incantation to suppress garbage output in Travis (see https://forum.arduino.cc/index.php?topic=469428.msg4217390#msg4217390)
sudo iptables -P INPUT DROP
sudo iptables -P FORWARD DROP
sudo iptables -P OUTPUT ACCEPT
sudo iptables -A INPUT -i lo -j ACCEPT
sudo iptables -A OUTPUT -o lo -j ACCEPT
sudo iptables -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

# run the tests
cd ${SOURCE_DIR}
./scripts/verify_samples.sh

