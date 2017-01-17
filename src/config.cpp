#include <EEPROM.h>

#include "debug.h"
#include "config.h"

#define STORAGE_SIZE 4096

#ifndef HOSTNAME
#define HOSTNAME "esplug"
#endif

#define wifiClientSsid_LONG_NAME      "wifiClientSsid"
#define wifiClientPassword_LONG_NAME  "wifiClientPassword"
#define wifiHostname_LONG_NAME        "wifiHostname"

#define wifiClientSsid_SHORT_NAME      "wcs"
#define wifiClientPassword_SHORT_NAME  "wcp"
#define wifiHostname_SHORT_NAME        "whn"

ConfigClass::ConfigClass() :
  modified(false)
{
  setDefaults();
}

void ConfigClass::begin()
{
  EEPROM.begin(STORAGE_SIZE);
  char start;
  uint8_t a, b;
  EEPROM.get(0, a);
  EEPROM.get(1, b);
  int length = a | (b << 8);
  EEPROM.get(2, start);

  DBUGF("Got %d %c from EEPROM", length, start);

  if(2 <= length && length < STORAGE_SIZE &&
    '{' == start)
  {
    char json[length + 1];
    for(int i = 0; i < length; i++) {
      json[i] = EEPROM.read(2+i);
    }
    json[length] = '\0';
    DBUGF("Found stored JSON %s", json);
    deserialize(json);
    modified = false;
  } else {
    DBUGF("No existing config found, writing factory settings");
    reset();
  }
}

void ConfigClass::reset()
{
  modified = true;
  setDefaults();
  Trigger();
  commit();
}

void ConfigClass::commit()
{
  if(false == modified) {
    return;
  }

  DBUGF("Saving config");

  String jsonStr;
  serialize(jsonStr, false);
  const char *json = jsonStr.c_str();
  DBUGF("Writing %s to EEPROM", json);
  int length = jsonStr.length();
  EEPROM.put(0, length & 0xff);
  EEPROM.put(1, (length >> 8) & 0xff);
  for(int i = 0; i < length; i++) {
    EEPROM.write(2+i, json[i]);
  }

  DBUGF("%d bytes written to EEPROM, committing", length + 2);

  EEPROM.commit();
  DBUGF("Done");

  modified = false;
}

void ConfigClass::setDefaults()
{
  wifiClientSsid = "";
  wifiClientPassword = "";
  wifiHostname = HOSTNAME;
}

bool ConfigClass::serialize(Print& json, bool longNames)
{
  DynamicJsonBuffer buffer;
  JsonObject& root = buffer.createObject();

  if(serialize(root, longNames))
  {
    root.printTo(json);
    return true;
  }

  return false;
}

bool ConfigClass::serialize(String& json, bool longNames)
{
  DynamicJsonBuffer buffer;
  JsonObject& root = buffer.createObject();

  if(serialize(root, longNames))
  {
    root.printTo(json);
    return true;
  }

  return false;
}

#define SET_VALUE(val) \
  root[(longNames ? val ## _LONG_NAME : val ## _SHORT_NAME)] = val

bool ConfigClass::serialize(JsonObject& root, bool longNames)
{
  SET_VALUE(wifiClientSsid);
  SET_VALUE(wifiClientPassword);
  SET_VALUE(wifiHostname);

  return true;
}

#undef SET_VALUE

#define GET_VALUE(val) do { \
  if(root.containsKey(val ## _LONG_NAME)) { \
    set_ ## val(root[val ## _LONG_NAME]); \
  } else if(root.containsKey(val ## _SHORT_NAME)) { \
    set_ ## val(root[val ## _SHORT_NAME]); \
  }} while(false)
#define GET_VALUE_AS_STRING(val) do { \
  if(root.containsKey(val ## _LONG_NAME)) { \
    set_ ## val(root[val ## _LONG_NAME].asString()); \
  } else if(root.containsKey(val ## _SHORT_NAME)) { \
    set_ ## val(root[val ## _SHORT_NAME].asString()); \
  }} while(false)

bool ConfigClass::deserialize(JsonObject& root)
{
  if(root.success())
  {
    GET_VALUE_AS_STRING(wifiClientPassword);
    GET_VALUE_AS_STRING(wifiClientSsid);
    GET_VALUE_AS_STRING(wifiHostname);

    return true;
  }

  return false;
}

#undef GET_VALUE
#undef GET_VALUE_AS_STRING

#define SET_VALUE(type, val) \
bool ConfigClass::set_ ## val (type value) \
{ \
  DBUG(val ## _LONG_NAME ": "); \
  DBUG(val); \
  DBUG(" != "); \
  DBUGLN(value); \
  if(val != value) { \
    DBUGLN("Value changed!"); \
    val = value; \
    setModified(); \
  } \
}

SET_VALUE(String, wifiClientPassword);
SET_VALUE(String, wifiClientSsid);
SET_VALUE(String, wifiHostname);

#undef SET_VALUE

ConfigClass Config;
