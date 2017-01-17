#ifndef __ESP_OTA_H
#define __ESP_OTA_H

#include <Arduino.h>

#include <MicroTasks.h>
#include <MicroTasksTask.h>

class EspOtaTask : public MicroTasks::Task
{
private:
  const char *hostName;
public:
  EspOtaTask(const char *hostName);
  EspOtaTask();

  void setHostName(String hostname);

  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);
};

#endif // __ESP_OTA_H
