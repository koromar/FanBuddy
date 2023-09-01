# FanBuddy
Fanbuddy is a non-destructive addon for the Sunlu S2 Filament Dryer to help extract humidity.  

A D1 MiniPro is placed with other electronics in a 3d-printed case. The Arduino Code sends humidity and temperature data to a MQTT Server. The fan can also be turned on and off via MQTT which enables it for Home-Assistant Automations based on the sensor data. 

This is the home for the arduino code and the home-assistant entities.   

Details can be found here: https://www.printables.com/model/564982  

![fanbuddy-schematic.png](pics%2Ffanbuddy-schematic.png)


# This repo contains
### `fanbuddy_arduino_code.ino`

Fanbuddy uses the following additional Libraries:  
* [Adafruit_HTU21DF_Library](https://github.com/adafruit/Adafruit_HTU21DF_Library)
* [ArduinoOTA](https://github.com/jandrassy/ArduinoOTA)
* [esp8266_mdns](https://github.com/mrdunk/esp8266_mdns)
* [EspMQTTClient](https://github.com/plapointe6/EspMQTTClient)

### `ha-entities.yaml`
This file contains the sensor and switch entities for home-assistant.  
Currently no auto-discovery towards home-assistant is implemented.