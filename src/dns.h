#ifndef __DNS_H
#define __DNS_H

#include <MicroTasks.h>
#include <MicroTasksTask.h>
#include <DNSServer.h>                // Required for captive portal

#include "dns.h"
#include "debug.h"

#define DNS_PORT 53

typedef void (* WiFiScanComplete)(int numberNetworks, void *pvData);

class DnsTask : public MicroTasks::Task
{
private:
  IPAddress apIP;
  DNSServer dnsServer;                      // Create class DNS server, captive portal re-direct
public:
  DnsTask() :
    dnsServer(),
    apIP(192, 168, 4, 1),
    MicroTasks::Task()
  {
  }

  void setup() {
    // Setup the DNS server redirecting all the domains to the apIP
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);
  }
  unsigned long loop(MicroTasks::WakeReason reason) {
    dnsServer.processNextRequest(); // Captive portal DNS re-dierct
  }
  void setIp(IPAddress &apIP) {
    this->apIP = apIP;
  }
};

#endif //  __WIFI_DNS_H
