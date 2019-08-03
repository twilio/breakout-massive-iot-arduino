# Twilio Breakout Massive SDK for Arduino

Breakout SDK enables developers to exchange data between low-powered devices and IoT cloud services. Breakout makes it easy to adopt low power optimizations provided by the Narrowband network and cellular modem hardware.

This SDK is primary intended for use with our Massive IoT Developer Kits but can likely also be used on similar boards with Arduino support with modification.

The current Twilio Massive IoT Developer Kit, version "Alfa" is based on a Wio factor development board with numerous Grove connectors and a Narrowband capable ublox SARA-N410 cellular module.  The developer kit has thusfar been distributed at [SIGNAL](https://www.twilio.com/signal) and comes with Grove sensors – humidity, light, and ultrasonic distance.

It is important to note that you must have a Narrowband SIM to utilize this kit, a standard LTE SIM will not be able to connect to the network.

This Arduino compatible library is backed by a more generic cellular module abstraction layer, [Twilio Breakout Massive SDK](https://github.com/twilio/Breakout_Massive_SDK), which also has support for the Quectel BG96 NB-IoT module.  The goal of this module is to make packet based communications easier when dealing with these cellular modules.

## Setting up your development environment
1. [Visit the Arduino IDE setup guide](IDESETUP.md) to learn how to set up your development environment.
2. [Visit the Alfa Developer Kit Quickstart](https://www.twilio.com/docs/wireless/quickstart/alfa-developer-kit) to learn how to register and activate your Narrowband SIM in the [Programmable Wireless Console](https://twilio.com/console/wireless/sims).

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
