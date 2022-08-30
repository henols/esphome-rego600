# esphome-rego600
ESPHome custom component for Rego 600 heat pump controller, version , running on ESP8266.

## ESPHome version
The current version in main is tested with ESPHome version `2022.3.0`. Make sure your ESPHome version is up to date if you experience compile problems.



ESPHome component for Rego 600 heat pump controller, found in:
* IVT Greenline
Might work with 
* Bosch xxxx
* Others

# Connection
Basic connection schematics (from http://rago600.sourceforge.net/)
![Generic connection](generic_com.gif)


## Installation
Clone the repository and create a companion `secrets.yaml` file with the following fields:
```
wifi.networks.ssid: <your wifi SSID>
wifi.networks.password: <your wifi password>
wifi.ap.password: <fallback AP password>
api.password: <the Home Assistant API password>
ota.password: <The OTA password>
```
Make sure to place the `secrets.yaml` file in the root path of the cloned project. The `wifi.ap.password` and `ota.password` fields can be set to any password before doing the initial upload of the firmware.



Prepare the microcontroller with ESPHome before you connect it to the circuit:
- Install the `esphome` [command line tool](https://esphome.io/guides/getting_started_command_line.html)
- Plug in the microcontroller to your USB port and run `esphome rego600.yaml run` to flash the firmware
- Remove the USB connection and connect the microcontroller to the rest of the circuit and plug it into the heat pump port.
- If everything works, your Home Assistant will now auto detect your new ESPHome integration.

You can check the logs by issuing `esphome rego600.yaml logs` (or use the super awesome ESPHome dashboard available as a Hass.io add-on or standalone). The logs should output data similar to this every 10 seconds when using `DEBUG` loglevel:

# References
* https://husdata.se/Download/o309fq2jio/H1%20Manual%2020.01.pdf
* http://rago600.sourceforge.net/
* https://github.com/dala318/esphome-rego600wifi.networks.ssid