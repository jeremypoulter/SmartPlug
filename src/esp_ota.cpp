#include <Arduino.h>
#include <ArduinoOTA.h>               // local OTA update from Arduino IDE

#include "debug.h"
#include "esp_ota.h"

EspOtaTask::EspOtaTask(const char *hostName) :
  hostName(hostName),
  MicroTasks::Task()
{
}

EspOtaTask::EspOtaTask() :
  hostName(NULL),
  MicroTasks::Task()
{
}

void EspOtaTask::setHostName(String hostname) {
  hostName = hostname.c_str();
  ArduinoOTA.setHostname(hostName);
}

void EspOtaTask::setup()
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostName);

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    static int lastProgress = 0;
    if(percent != lastProgress) {
      Serial.printf("Progress: %u%%\r", percent);
      lastProgress = percent;
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  // Start local OTA update server
  ArduinoOTA.begin();
}

unsigned long EspOtaTask::loop(MicroTasks::WakeReason reason)
{
  ArduinoOTA.handle();
  return 0;
}
