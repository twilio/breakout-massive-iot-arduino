
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
> When logging, the additional `L_CLI` level ensure that the output will always be visible, no matter the set level.

## Limitations and Workarounds
1. U-Blox UDP datagram size limitation
    *  **Problem:** The U-Blox cellular module can receive up to 512 bytes of data in hex mode. Hence this is a limitation for UDP datagrams.
    *  **Solution:** partial solution would be to switch from the hex mode to binary mode. This shall double the amount of data received, yet it makes the code much more complex. So far 512 was a good upper limit, especially given the NB-IoT targets here, hence this wasn't explored.
