# SmartPlug
Open source firmware for ESP8266 WiFi smart plugs such as [this one](http://www.banggood.com/Smart-Home-WiFi-Socket-Switch-UK-Plug-Wireless-Timer-Remote-Control-p-1022471.html?p=7U30132019453201507N) from [Banggood](https://www.banggood.com/?p=7U30132019453201507N).

## Features

* Web UI
* MQTT

## Install Dev ESP8266 core

The [Banggood SmartPlug](http://www.banggood.com/Smart-Home-WiFi-Socket-Switch-UK-Plug-Wireless-Timer-Remote-Control-p-1022471.html?p=7U30132019453201507N) has a 40mhz crystal so needs a special version of the ESP8266 Arduino core.

'''
platformio platform install https://github.com/jeremypoulter/platform-espressif8266.git#feature/40mhz
'''
