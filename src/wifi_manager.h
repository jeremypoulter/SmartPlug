#ifndef __WIFI_MANAGER_H
#define __WIFI_MANAGER_H

#include <MicroTasks.h>
#include <MicroTasksTask.h>
#include <MicroTasksEvent.h>
#include <MicroTasksEventListener.h>

#include "dns.h"                // Required for captive portal

typedef void (* WiFiScanComplete)(int numberNetworks, void *pvData);

class WiFiScanCompleteEvent : public MicroTasks::Event
{
public:
  void ScanComplete() {
    Trigger();
  }
};

class WiFiManagerTask : public MicroTasks::Task
{
private:
  DnsTask dns;                      // Create class DNS server, captive portal re-direct
  WiFiScanCompleteEvent scanCompleteEvent;  //

  // Access Point SSID, password & IP address. SSID will be softAP_ssid + chipID to make SSID unique
  String softAP_ssid;
  String softAP_password;
  IPAddress apIP;
  IPAddress netMask;

  // WiFi Client
  String client_ssid;
  String client_password;
  String hostname;

  // Are we in client mode or AP
  bool client;
  long timeout;
  bool scan;

  // WiFi connection state cache
  int wifiState;

  // WiFi connection LED state
  int wifiLedState;

  void startAP();
  unsigned long loopAP();
  void stopAP();

  void startClient();
  unsigned long loopClient();
  void stopClient();
public:
  WiFiManagerTask(String hostname, String ssid, String password);
  WiFiManagerTask();

  void setHostName(String hostname);
  void setClientDetails(String ssid, String password);

  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);

  void StartScan();
  void onScanComplete(MicroTasks::EventListener& eventListener);
};

#endif //  __WIFI_MANAGER_H
