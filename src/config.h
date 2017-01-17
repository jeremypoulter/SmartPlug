#ifndef __CONFIG_H
#define __CONFIG_H

#include <MicroTasks.h>
#include <MicroTasksEvent.h>
#include <ArduinoJson.h>

class ConfigStorage;

class ConfigClass : public MicroTasks::Event
{
private:
  bool modified;

  unsigned long serialBaud;
  int serialConfig;

  String wifiClientSsid;
  String wifiClientPassword;
  String wifiHostname;

  void setDefaults();
  void setModified() {
    modified = true;
    Trigger();
  }

  bool set_serialBaud(unsigned long value);
  bool set_serialConfig(int value);
  bool set_wifiClientSsid(String value);
  bool set_wifiClientPassword(String value);
  bool set_wifiHostname(String value);
public:
  ConfigClass();

  void begin();
  void reset();
  void commit();

  bool serialize(String& json) {
    return serialize(json, true);
  }
  bool serialize(String& json, bool longNames);
  bool serialize(Print& json) {
    return serialize(json, true);
  }
  bool serialize(Print& json, bool longNames);
  bool serialize(JsonObject& root, bool longNames);

  bool deserialize(String& json) {
    return deserialize(json.c_str());
  }
  bool deserialize(const char *json) {
    DynamicJsonBuffer buffer;
    return deserialize(buffer.parseObject(json));
  }
  // Although this is the same code as above and you
  // can easily cast a char * to const char * ArduinoJson
  // has to make a copy of the string with a const char*
  bool deserialize(char *json) {
    DynamicJsonBuffer buffer;
    return deserialize(buffer.parseObject(json));
  }
  bool deserialize(JsonObject& root);

  uint32_t getSerialBaud() {
    return serialBaud;
  }
  bool setSerialBaud(uint32_t value) {
    set_serialBaud(value);
  }

  SerialConfig getSerialConfig() {
    return (SerialConfig)serialConfig;
  }
  bool setSerialConfig(SerialConfig value) {
    set_serialConfig(value);
  }

  String getWifiClientSsid() {
    return wifiClientSsid;
  }
  bool setWifiClientSsid(String value) {
    set_wifiClientSsid(value);
  }

  String getWifiClientPassword() {
    return wifiClientPassword;
  }
  bool setWifiClientPassword(String value) {
    set_wifiClientPassword(value);
  }

  String getWifiHostname() {
    return wifiHostname;
  }
  bool setWifiHostname(String value) {
    set_wifiHostname(value);
  }
};

extern ConfigClass Config;

#endif //  __CONFIG_H
