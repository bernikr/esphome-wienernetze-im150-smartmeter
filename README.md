# esphome-wienernetze-im150-smartmeter

This is a custom component for ESPHome to integrate a Siemens IM150 Smartmeter provided by Wiener Netze into Home Assistant via the infrared "Kundenschnittstelle". It gives power and energy readings, both positive and negative for active as well as reactive power.

## Installation and usage
Copy both the im150.cpp and the im150.h file into a folder accessible to ESPHome and create a config based on [the example](example.yaml).
Dont forget to add your encryption key from the smartmeter website otherwise it will not work.

## Tested Hardware
This component shoud work on all ESP8266 and ESP32 microcontrollers with and IR read-head attached to them. I used a ready made read-write head with an ESP01s built in that I got for 30€ on [ebay](https://www.ebay.de/itm/275501110235).

## Thanks
The following resources were much help in the development of this component:
- https://github.com/aldadic/esp32-smartmeter-reader
- https://github.com/aburgr/smartmeter-reader
- https://github.com/DomiStyle/esphome-dlms-meter
- https://www.lteforum.at/mobilfunk/wiener-netze-smart-meter-auslesen.16222/
