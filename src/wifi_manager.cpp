#include <Arduino.h>
#include <ESP8266WiFi.h>              // Connect to Wifi

#include "wifi_manager.h"
#include "dns.h"
#include "debug.h"

#ifndef WIFI_CONNECTION_LED
#define WIFI_CONNECTION_LED       0
#endif
#ifndef WIFI_CONNECTION_LED_STATE
#define WIFI_CONNECTION_LED_STATE LOW
#endif

WiFiManagerTask::WiFiManagerTask(String hostname, String ssid, String password) :
  scanCompleteEvent(),
  softAP_ssid(hostname+"_"+String(ESP.getChipId())),
  softAP_password(""),
  apIP(192, 168, 4, 1),
  netMask(255, 255, 255, 0),
  client_ssid(ssid),
  client_password(password),
  hostname(hostname),
  client(false),
  timeout(millis()),
  scan(false),
  wifiState(-1),
  wifiLedState(!WIFI_CONNECTION_LED_STATE),
  dns(),
  MicroTasks::Task()
{
}

WiFiManagerTask::WiFiManagerTask() :
  scanCompleteEvent(),
  softAP_ssid("_"+String(ESP.getChipId())),
  softAP_password(""),
  apIP(192, 168, 4, 1),
  netMask(255, 255, 255, 0),
  client_ssid(""),
  client_password(""),
  hostname(""),
  client(false),
  timeout(millis()),
  scan(false),
  wifiState(-1),
  wifiLedState(!WIFI_CONNECTION_LED_STATE),
  dns(),
  MicroTasks::Task()
{
}

void WiFiManagerTask::setHostName(String hostname) {
  this->hostname = hostname;
  this->softAP_ssid = hostname+"_"+String(ESP.getChipId());
}

void WiFiManagerTask::setClientDetails(String ssid, String password) {
  client_ssid = ssid;
  client_password = password;
}

void WiFiManagerTask::setup()
{
  DBUGF("hostname = '%s'", hostname.c_str());
  DBUGF("client_ssid = '%s'", client_ssid.c_str());
  DBUGF("client_password = '%s'", client_password.c_str());

  pinMode(WIFI_CONNECTION_LED, OUTPUT);
  digitalWrite(WIFI_CONNECTION_LED, wifiLedState);

  if(client_ssid != "") {
    startClient();
  } else {
    startAP();
  }
}

unsigned long WiFiManagerTask::loop(MicroTasks::WakeReason reason)
{
  if(scan)
  {
    int n = WiFi.scanComplete();
    if(WIFI_SCAN_RUNNING != n)
    {
      DBUGF("Complete, found %d", n);
      scanCompleteEvent.ScanComplete();
      scan = false;
    } else {
      DBUGF("Scanning... (%d)", n);
    }
  }

  return client ? loopClient() : loopAP();
}

void WiFiManagerTask::startAP()
{
  DBUGF("Starting AP %s, pass %s", softAP_ssid.c_str(), softAP_password.c_str());
  WiFi.disconnect();

  // Start up the AP
  WiFi.softAPConfig(apIP, apIP, netMask);
  WiFi.softAP(softAP_ssid.c_str(), softAP_password.c_str());

  // Start the DNS task
  dns.setIp(apIP);
  MicroTask.startTask(dns);

  client = false;
}

unsigned long WiFiManagerTask::loopAP()
{
  wifiLedState = !wifiLedState;
  digitalWrite(WIFI_CONNECTION_LED, wifiLedState);

  if(client_ssid == "") {
    // No client SSID set, nothing to do..
    return 1000;
  }

  // Scan for the client SSID
  int n = WiFi.scanComplete();
  if(WIFI_SCAN_RUNNING != n)
  {
    DBUGF("%d networks found", n);
    for (int i = 0; i < n; ++i) {
      if(client_ssid == WiFi.SSID(i)) {
        startClient();
        return 0;
      }
    }

    StartScan();
  }

  return 1000;
}

void WiFiManagerTask::stopAP()
{
}

void WiFiManagerTask::startClient()
{
  DBUGF("Connecting to %s, pass %s", client_ssid.c_str(), client_password.c_str());
  WiFi.disconnect();
  WiFi.hostname(hostname);
  WiFi.begin(client_ssid.c_str(), client_password.c_str());
  client = true;
  timeout = millis() + (30 * 1000);
}

unsigned long WiFiManagerTask::loopClient()
{
  //DBUGF("Loop Client");

  if(WL_CONNECTED == WiFi.status())
  {
    if(wifiState != WL_CONNECTED) {
      DBUGLN("Connected");
      wifiLedState = WIFI_CONNECTION_LED_STATE;
      digitalWrite(WIFI_CONNECTION_LED, wifiLedState);
      wifiState = WL_CONNECTED;
    }

    // We are connected nothing to do...
    return 1000;
  }

  wifiState = WiFi.status();
  wifiLedState = !wifiLedState;
  digitalWrite(WIFI_CONNECTION_LED, wifiLedState);

  if(millis() > timeout) {
    // timeout connecting to AP
    startAP();
  }

  return 500;
}

void WiFiManagerTask::stopClient()
{
}

void WiFiManagerTask::StartScan()
{
  WiFi.scanNetworks(true);
  scan = true;
  DBUGF("Start scan");
}

void WiFiManagerTask::onScanComplete(MicroTasks::EventListener& eventListener)
{
  scanCompleteEvent.Register(&eventListener);
}
