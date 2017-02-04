#ifndef __WEB_UI_H
#define __WEB_UI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <MicroTasks.h>
#include <MicroTasksTask.h>
#include <MicroTasksEventListener.h>

#include "wifi_manager.h"

class WebUiTask;
class WebUiTask : public MicroTasks::Task
{
private:
  static WebUiTask *self;

  AsyncWebServer server;
  AsyncWebSocket ws;
  MicroTasks::EventListener scanCompleteEvent;
  WiFiManagerTask &wifi;

  AsyncWebServerRequest *scanRequest;

  bool reboot;
  bool enableCors;

  static void onNotFound(AsyncWebServerRequest *request);

  static void handleCors(AsyncWebServerResponse *response);
  static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

public:
  WebUiTask(WiFiManagerTask &wifi);
  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

#endif // __WEB_UI_H
