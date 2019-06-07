
# Twilio Breakout SDK for Arduino
This page documents how to get started using the Breakout SDK and what it provides. Today, the Breakout SDK is built for the STM32F405RG MCU and U-Blox Sara-N410 cellular module in mind. This specific Developer Board was provided in Twilio's Alfa Developer Kit and distributed to [SIGNAL 2018](https://www.twilio.com/signal) attendees, and it came with Grove sensors – humidity, light, and ultrasonic.

## Setting up your development environment
1. [Visit the Arduino IDE setup guide](IDESETUP.md) to learn how to set up your development environment.
2. [Visit the Alfa Developer Kit Quickstart](https://www.twilio.com/docs/wireless/quickstart/alfa-developer-kit) to learn how to register and activate your Narrowband SIM in the [Programmable Wireless Console](https://twilio.com/console/wireless/sims).

## Breakout SDK

Breakout SDK enables developers to exchange data between low-powered devices and IoT cloud services. Breakout makes it easy to adopt low power optimizations provided by the Narrowband network and cellular modem hardware.

### Set log level
Owl log provides robust output and color coding to Serial output. 
Log verbosity from most critical to most verbose:
* errors
	* `L_ERR`
* warnings
	* `L_WARN`
* information 
	* `L_INFO`
* debug
	* `L_DBG`
```
 owl_log_set_level(L_DBG);
```
> When logging, the additional L_CLI level ensure that the output will always be visible, no matter the set level.

## Limitations and Workarounds
1. U-Blox data size limitation
    *  **Problem:** The U-Blox cellular module can receive up to 512 bytes of data in hex mode. Hence this is a limitation for UDP datagrams.
    *  **Solution:** partial solution would be to switch from the hex mode to binary mode. This shall double the amount of data received, yet it makes the code much more complex. So far 512 was a good upper limit, especially given the NB-IoT targets here, hence this wasn't explored.
2. Arduino Seeed STM32F4 Boards Package - USART buffer limitation
    *  **Problem:** The Seeed STM32F4 Boards package, published through the Boards Manager system, has a limitation in the USART interface implementation which we're using to communicate with the modem. The default USART_RX_BUF_SIZE is set to 256 bytes. With overheads and hex mode, this reduces the maximum chunk of data which we can read from the modem quite significantly. The effect is that the modem data in the receive side is shifted over its beginning, such that only the last 255 bytes are actually received by OwlModem.
    *  **Solution:** see [Update USART buffer below](#update-usart-buffer).

### Update USART buffer
This is **required** for Wio STM32F4 devices not on 1.2.3.
1. Locate ``/libmaple/usart.h``
    * OSX:  ``/Users/{{UserNameHere}}/Library/Arduino15/packages/Seeeduino/hardware/Seeed_STM32F4/{{VersionHere}}/cores/arduino/libmaple/usart.h``
    * Linux:
    ``~/.arduino15/packages/Seeeduino/hardware/Seeed_STM32F4/{{VersionHere}}/cores/arduino/libmaple/usart.h``
    * Windows:  ``C:/Users/{{UserNameHere}}/Documents/Arduino/hardware/Seeed_STM32F4/{{VersionHere}}/cores/arduino/libmaple/usart.h``
2. Update ``USART_RX_BUF_SIZE`` to ``1280``
3. Update ``USART_TX_BUF_SIZE`` to ``1280``
