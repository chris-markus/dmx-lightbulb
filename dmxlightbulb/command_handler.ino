// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "command.h"
#include "settings.h"

bool setBoolSetting(void* setting, const char* value) {
  bool *_setting = (bool*)(setting);
  if (strcmp(value, "1") == 0) {
    *_setting = true;
  }
  else if (strcmp(value, "0") == 0){
    *_setting = false;
  }
  else {
    return false;
  }
  return true;
}

bool numericSettingHelper(void* setting_v, const char* val_str, int highVal, int lowVal) {
  int *_setting = (int*)(setting_v);
  int val = strtol(val_str, NULL, 10);
  if (val <= highVal && val >= lowVal) {
    *_setting = val;
    return true;
  }
  return false;
}

bool setDMXAddress(void* setting, const char* value) {
  int high = DMX_MAX_ADDRESS; 
  int low = 1;
  return numericSettingHelper(setting, value, high, low);
}

bool setDMXUniverse(void* setting, const char* value) {
  int high = DMX_MAX_UNIVERSE;
  int low = 1;
  return numericSettingHelper(setting, value, high, low);
}

bool setColorSetting(void* setting, const char* value) {
  String colorString(value);
  Settings.artnetEnable = false;
  Settings.sACNEnable = false;
  if (colorString.length() >= 8) {
    int start = 0;
    if(colorString.substring(0,1) == "#") {
      start = 1;
    }
    unsigned long colorVal = strtoll(colorString.substring(start).c_str(), NULL, 16);
    //Serial.println(colorString.substring(start));
    //Serial.println(colorVal);
    setLEDColor((uint8_t)((colorVal>>((unsigned long)(24)))), (uint8_t)((colorVal&0xFF0000)>>16), (uint8_t)((colorVal&0xFF00)>>8), (uint8_t)(colorVal&0xFF));
    //Serial.print("Setting Color: ");
    //Serial.println((uint8_t)((colorVal>>24)));
    return true;
  }
  return false;
}

Command commands[] = {
  {
    "artnetEnable",
    (void*)(&Settings.artnetEnable),
    setBoolSetting,
  },
  {
    "sACNEnable",
    (void*)(&Settings.sACNEnable),
    setBoolSetting,
  },
  {
    "arduinoOTAEnable",
    (void*)(&Settings.arduinoOTAEnable),
    setBoolSetting,
  },
  {
    "color",
    NULL,
    setColorSetting,
  },
  {
    "dmxAddress",
    (void*)(&Settings.DMXAddress),
    setDMXAddress,
  },
  {
    "dmxUniverse",
    (void*)(&Settings.DMXUniverse),
    setDMXUniverse,
  }
};

//Command artnetEnable(F("artnetEnable"), 2, ["0", "1"], setArtnetEnable);
//Command sACNEnable(F("sACNEnable"), 2, ["0", "1"], setsACNEnable)
/*
bool setArtnetEnable(String& value) {
  if (server.arg(i) == F("1")) {
    Settings.artnetEnable = true;
  }
  else {
    Settings.artnetEnable = false;
  }
}

bool setsACNEnable(String& value) {
  if (server.arg(i) == F("1")) {
    Settings.sACNEnable = true;
  }
  else {
    Settings.sACNEnable = false;
  }
}*/