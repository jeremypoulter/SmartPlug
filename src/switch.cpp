#include <Arduino.h>

#include "switch.h"
#include "debug.h"

#ifndef SWITCH_RELAY_ON_STATE
#define SWITCH_RELAY_ON_STATE LOW
#endif
#ifndef SWITCH_LED_ON_STATE
#define SWITCH_LED_ON_STATE LOW
#endif

SwitchTask::SwitchTask(int ledPin, int buttonPin, int relayPin) :
  ledPin(ledPin),
  buttonPin(buttonPin),
  relayPin(relayPin),
  buttonState(LOW),
  switchState(LOW)
{
}

void SwitchTask::setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  buttonState = digitalRead(buttonPin);
  digitalWrite(ledPin, !switchState);
  digitalWrite(relayPin, switchState);
}

unsigned long SwitchTask::loop(MicroTasks::WakeReason reason)
{
  int newState = digitalRead(buttonPin);
  if(newState != buttonState)
  {
    buttonState = newState;
    if(LOW == buttonState) {
      setSwitchState(!switchState);
    }

    // Don't check again for a bit to help with debounce
    return 20;
  }

  return 0;
}

void SwitchTask::setSwitchState(int state)
{
  if(state != switchState)
  {
    DBUGF("Turning switch %s", state ? "on" : "off");
    switchState = state;
    digitalWrite(ledPin, switchState ? SWITCH_LED_ON_STATE : !SWITCH_LED_ON_STATE);
    digitalWrite(relayPin, switchState ? SWITCH_RELAY_ON_STATE : !SWITCH_RELAY_ON_STATE);
    Trigger();
  }
}
