#ifndef __SWITCH_H
#define __SWITCH_H

#include <Arduino.h>

#include <MicroTasks.h>
#include <MicroTasksTask.h>
#include <MicroTasksEvent.h>
#include <MicroTasksEventListener.h>

class SwitchTask : public MicroTasks::Task, public MicroTasks::Event
{
private:
  int buttonState;
  int switchState;

  int ledPin;
  int buttonPin;
  int relayPin;
public:
  SwitchTask(int ledPin, int buttonPin, int relayPin);

  void setup();
  unsigned long loop(MicroTasks::WakeReason reason);

  void setSwitchState(int state);
  int getSwitchState() {
    return switchState;
  }
  void onSwitchStateChange(MicroTasks::EventListener& eventListener) {
    Register(&eventListener);
  }
};

#endif // __ESP_OTA_H
