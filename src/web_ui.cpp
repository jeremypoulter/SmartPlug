#include <Arduino.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#include "debug.h"
#include "web_ui.h"
#include "config.h"

WebUiTask *WebUiTask::self = NULL;

WebUiTask::WebUiTask(WiFiManagerTask &wifi) :
  server(80),
  ws("/ws"),
  scanCompleteEvent(this),
  wifi(wifi),
  reboot(false),
  enableCors(true),
  MicroTasks::Task()
{
  wifi.onScanComplete(scanCompleteEvent);

  self = this;
}

void WebUiTask::setup()
{
  MDNS.addService("http", "tcp", 80);

  // Setup the static files
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    handleCors(response);

    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["id"] = ESP.getChipId();
    root["heap"] = ESP.getFreeHeap();
    root["version"] = ESCAPEQUOTE(VERSION);

    root.printTo(*response);
    request->send(response);
  });

  server.on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if(NULL == self->scanRequest)
    {
      DBUGF("Start WiFi scan");
      self->scanRequest = request;
      self->wifi.StartScan();
    }
    else
    {
      request->send(400, "application/json", "{\"msg\":\"Busy\"}");
    }
  });

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    handleCors(response);

    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["mac"] = WiFi.macAddress();
    root["localIP"] = WiFi.localIP().toString();
    root["subnetMask"] = WiFi.subnetMask().toString();
    root["gatewayIP"] = WiFi.gatewayIP().toString();
    root["dnsIP"] = WiFi.dnsIP().toString();
    wl_status_t status = WiFi.status();
    root["status"] = WL_NO_SHIELD == status ? "WL_NO_SHIELD" :
                     WL_IDLE_STATUS == status ? "WL_IDLE_STATUS" :
                     WL_NO_SSID_AVAIL == status ? "WL_NO_SSID_AVAIL" :
                     WL_SCAN_COMPLETED == status ? "WL_SCAN_COMPLETED" :
                     WL_CONNECTED == status ? "WL_CONNECTED" :
                     WL_CONNECT_FAILED == status ? "WL_CONNECT_FAILED" :
                     WL_CONNECTION_LOST == status ? "WL_CONNECTION_LOST" :
                     WL_DISCONNECTED == status ? "WL_DISCONNECTED" :
                     "UNKNOWN";
    root["hostname"] = WiFi.hostname();
    root["SSID"] = WiFi.SSID();
    root["BSSID"] = WiFi.BSSIDstr();
    root["RSSI"] = WiFi.RSSI();

    root.printTo(*response);
    request->send(response);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    handleCors(response);
    Config.serialize(*response);
    request->send(response);
  });

  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    int params = request->params();
    DBUGF("Save settings, %d params", params);
    for(int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        DBUGF("_FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        DBUGF("_POST[%s]: %s", p->name().c_str(), p->value().c_str());
      } else {
        DBUGF("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
      }
    }

    if(request->hasParam("body", true))
    {
      AsyncWebParameter* p = request->getParam("body", true);
      String json = p->value();
      if(Config.deserialize(json))
      {
        Config.commit();

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{'msg':'done'}");
        handleCors(response);
        request->send(response);
      }
      else
      {
        AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{'msg':'Could not parse JSON'}");
        handleCors(response);
        request->send(response);
      }
    }
    else
    {
      AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{'msg':'No body'}");
      handleCors(response);
      request->send(response);
    }
  });

  server.on("/settings", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    Config.reset();
    self->reboot = true;
    MicroTask.wakeTask(self);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{'msg':'done'}");
    handleCors(response);
    request->send(response);
  });

  server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{'msg':'done'}");
    handleCors(response);
    request->send(response);

    self->reboot = true;
    MicroTask.wakeTask(self);
  });

  server.onNotFound(onNotFound);

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
  {
    if(!index) {
      DBUGF("BodyStart: %u", total);
    }
    DBUGF("%s", (const char*)data);
    if(index + len == total) {
      DBUGF("BodyEnd: %u", total);
    }
  });

  server.begin();
}

unsigned long WebUiTask::loop(MicroTasks::WakeReason reason)
{
  if(WakeReason_Event == reason)
  {
    int n = WiFi.scanComplete();
    if(NULL != WebUiTask::scanRequest && n != WIFI_SCAN_RUNNING)
    {
      DBUGF("WiFi scan complete");

      AsyncResponseStream *response = WebUiTask::scanRequest->beginResponseStream("application/json");
      handleCors(response);
      DynamicJsonBuffer jsonBuffer;

      JsonArray& root = jsonBuffer.createArray();
      DBUGF("%d networks found", n);
      for (int i = 0; i < n; ++i)
      {
        JsonObject& ssid = jsonBuffer.createObject();
        ssid["ssid"] = WiFi.SSID(i);
        int enc = WiFi.encryptionType(i);
        ssid["encryptionType"] = ENC_TYPE_WEP == enc ? "ENC_TYPE_WEP" :
                                 ENC_TYPE_TKIP == enc ? "ENC_TYPE_TKIP" :
                                 ENC_TYPE_CCMP == enc ? "ENC_TYPE_CCMP" :
                                 ENC_TYPE_NONE == enc ? "ENC_TYPE_NONE" :
                                 ENC_TYPE_AUTO == enc ? "ENC_TYPE_AUTO" :
                                 "UNKNOWN";
        ssid["rssi"] = WiFi.RSSI(i);
        root.add(ssid);
      }

      root.printTo(*response);
      WebUiTask::scanRequest->send(response);
      WebUiTask::scanRequest = NULL;
    }
  }

  if(reboot) {
    if(WakeReason_Manual == reason) {
      Serial.println("Rebooting...");
      return 100;
    }
    ESP.restart();
  }

  return MicroTask.Infinate;
}

void WebUiTask::onNotFound(AsyncWebServerRequest *request)
{
  DBUG("NOT_FOUND: ");
  if(request->method() == HTTP_GET) {
    DBUGF("GET");
  } else if(request->method() == HTTP_POST) {
    DBUGF("POST");
  } else if(request->method() == HTTP_DELETE) {
    DBUGF("DELETE");
  } else if(request->method() == HTTP_PUT) {
    DBUGF("PUT");
  } else if(request->method() == HTTP_PATCH) {
    DBUGF("PATCH");
  } else if(request->method() == HTTP_HEAD) {
    DBUGF("HEAD");
  } else if(request->method() == HTTP_OPTIONS) {
    DBUGF("OPTIONS");
  } else {
    DBUGF("UNKNOWN");
  }
  DBUGF(" http://%s%s", request->host().c_str(), request->url().c_str());

  if(request->contentLength()){
    DBUGF("_CONTENT_TYPE: %s", request->contentType().c_str());
    DBUGF("_CONTENT_LENGTH: %u", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for(i=0; i<headers; i++) {
    AsyncWebHeader* h = request->getHeader(i);
    DBUGF("_HEADER[%s]: %s", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for(i = 0; i < params; i++) {
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){
      DBUGF("_FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      DBUGF("_POST[%s]: %s", p->name().c_str(), p->value().c_str());
    } else {
      DBUGF("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void WebUiTask::handleCors(AsyncWebServerResponse *response)
{
  if(self->enableCors) {
    response->addHeader("Access-Control-Allow-Origin", "*");
  }
}
