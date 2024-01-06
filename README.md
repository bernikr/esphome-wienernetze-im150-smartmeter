# esphome-wienernetze-im150-smartmeter

This is a custom component for ESPHome to integrate a Siemens IM150 Smartmeter provided by Wiener Netze into Home Assistant via the infrared "Kundenschnittstelle". It gives power and energy readings, both positive and negative for active as well as reactive power.

## Installation and usage

Add the following code to your yaml file.
Dont forget to add your encryption key from the smartmeter website otherwise it will not work.

```
external_components:
  - source: github://bernikr/esphome-wienernetze-im150-smartmeter

logger:
  baud_rate: 0

uart:
  tx_pin: 1
  rx_pin: 3
  baud_rate: 9600

im150:
  key: <enter your key here>

sensor:
  - platform: im150
    active_energy_pos:
      name: Energy
    active_power_pos:
      name: Power
    active_energy_neg:
      name: Negative Energy
    active_power_neg:
      name: Negative Power
    reactive_energy_pos:
      name: Reactive Energy
    reactive_power_pos:
      name: Reactive Power
    reactive_energy_neg:
      name: Reactive Negative Energy
    reactive_power_neg:
      name: Reactive Negative Power

text_sensor:
  - platform: im150
    active_energy_pos:
      name: Energy Raw
    active_energy_neg:
      name: Negative Energy Raw
    reactive_energy_pos:
      name: Reactive Energy Raw
    reactive_energy_neg:
      name: Reactive Negative Energy Raw
```

For every sensor normal esphome sensor configs can be used to set name, id, icon, etc. or add filters and so on.

**Warning**  
All energy sensors roll over every 1000 kWh and start again from 0 due to precicion issues of esphome.
(Sensors are always 32bit floats, if the meter is t0o high the sensor cant update every Wh anymore.)  
This is not a problem when using the `active_energy_pos` sensor for the energy dashboard in Home Assistant as it is set to `total_increasing` and therefore Home Assistant knows that a drop from 1000 to 0 is a reset of the counter and not a negataive consumption of 1000kWh.

As an alternative you can use the text_sensors which always result in the full counter of the meter (in Wh not kWh) but using them further would require a bit post processing (probably a template sensor) in Home Assistant in order to use them as propper numeric sensors.

## Tested Hardware
This component shoud work on all ESP8266 and ESP32 microcontrollers with and IR read-head attached to them. I used a ready made read-write head with an ESP01s built in that I got for 30€ on [ebay](https://www.ebay.de/itm/275501110235).

## Thanks
The following resources were much help in the development of this component:
- https://github.com/aldadic/esp32-smartmeter-reader
- https://github.com/aburgr/smartmeter-reader
- https://github.com/DomiStyle/esphome-dlms-meter
- https://www.lteforum.at/mobilfunk/wiener-netze-smart-meter-auslesen.16222/
