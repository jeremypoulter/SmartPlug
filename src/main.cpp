/*
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <FS.h>

#include "debug.h"
#include "esp_ota.h"
#include "web_ui.h"
#include "wifi_manager.h"
#include "config.h"
#include "switch.h"

#ifndef POWER_LED
#define POWER_LED 14
#endif
#ifndef RELAY_COMTR
#define RELAY_COMTR 13
#endif
#ifndef BUTTON_PIN
#define BUTTON_PIN 5
#endif

EspOtaTask espOta;
WiFiManagerTask wifi;
SwitchTask switchTask(POWER_LED, BUTTON_PIN, RELAY_COMTR);
WebUiTask webUi(wifi, switchTask);

class ConfigManagerTask : public MicroTasks::Task
{
private:
  MicroTasks::EventListener configChangedEvent;

public:
  ConfigManagerTask() :
    configChangedEvent(this),
    MicroTasks::Task()
  {

  }

  void setup()
  {
    updateConfig();
    Config.Register(&configChangedEvent);
  }

  unsigned long loop(MicroTasks::WakeReason reason)
  {
    if(WakeReason_Event == reason) {
      updateConfig();
    }

    return MicroTask.WaitForEvent;
  }

protected:
  void updateConfig()
  {
    DBUGF("Update config");
    espOta.setHostName(Config.getWifiHostname());

    wifi.setHostName(Config.getWifiHostname());
    wifi.setClientDetails(Config.getWifiClientSsid(), Config.getWifiClientPassword());
  }
} configManager;

void setup() {
  DEBUG_BEGIN(115200);
  DBUGLN("");
  DBUGLN("");
  DBUGF("ESPlug %s", ESCAPEQUOTE(VERSION));
  DBUGLN("");

  SPIFFS.begin();
  Config.begin();

  MicroTask.startTask(switchTask);
  MicroTask.startTask(configManager);
  MicroTask.startTask(wifi);
  MicroTask.startTask(espOta);
  MicroTask.startTask(webUi);
}

void loop() {
  MicroTask.update();
}
