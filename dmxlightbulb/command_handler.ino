// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "command.h"
#include "settings.h"
#include "driver.h"

bool boolCmdHelper(void* setting, const char* value) {
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

bool numericCmdHelper(void* setting_v, const char* val_str, int highVal, int lowVal) {
  int *_setting = (int*)(setting_v);
  int val = strtol(val_str, NULL, 10);
  if (val <= highVal && val >= lowVal) {
    *_setting = val;
    return true;
  }
  return false;
}

bool cmd_DMXAddressHandler(void* setting, const char* value) {
  int high = DMX_MAX_ADDRESS; 
  int low = 1;
  return numericCmdHelper(setting, value, high, low);
}

bool cmd_DMXUniverseHandler(void* setting, const char* value) {
  int high = DMX_MAX_UNIVERSE;
  int low = 1;
  return numericCmdHelper(setting, value, high, low);
}

bool cmd_colorHandler(void* setting, const char* value) {
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

bool cmd_networkControlHandler(void* setting, const char* value) {
  if (strcmp(value, CMD_NETWORK_VALUE_ARTNET) == 0) {
    drv_artnet.setupFunc();
    Settings.artnetEnable = true;
    Settings.sACNEnable = false;
  }
  else if (strcmp(value, CMD_NETWORK_VALUE_SACN) == 0) {
    drv_sACN.setupFunc();
    Settings.artnetEnable = false;
    Settings.sACNEnable = true;
  }
  else if (strcmp(value, CMD_NETWORK_VALUE_NONE) == 0) {
    Settings.artnetEnable = false;
    Settings.sACNEnable = false;
  }
  else {
    return false;
  }
  return true;
}

bool cmd_arduinoOTAHandler(void* setting, const char* value) {
  bool retVal = boolCmdHelper(setting, value);
  if (retVal && Settings.arduinoOTAEnable) {
    drv_OTA.setupFunc();
  }
  return retVal;
}

bool cmd_saveColorHandler(void* setting, const char* value) {
  return boolCmdHelper(setting, value);
}

bool cmd_rebootHandler(void* setting, const char* value) {
  ESP.restart();
  return true;
}

Command cmd_networkControl = {
  CMD_NETWORK_NAME,
  NULL,
  cmd_networkControlHandler,
};

Command cmd_arduinoOTAEnable = {
  CMD_ARDUINOOTA_NAME,
  (void*)(&Settings.arduinoOTAEnable),
  cmd_arduinoOTAHandler,
};

Command cmd_color = {
  CMD_COLOR_NAME,
  NULL,
  cmd_colorHandler,
};

Command cmd_dmxAddress = {
  CMD_DMXADDRESS_NAME,
  (void*)(&Settings.DMXAddress),
  cmd_DMXAddressHandler,
};

Command cmd_dmxUniverse = {
  CMD_DMXUNIVERSE_NAME,
  (void*)(&Settings.DMXUniverse),
  cmd_DMXUniverseHandler,
};

Command cmd_saveColor = {
  CMD_SAVECOLOR_NAME,
  (void*)(&Settings.saveColor),
  cmd_saveColorHandler,
};

Command cmd_reboot = {
  CMD_REBOOT_NAME,
  NULL,
  cmd_rebootHandler,
};