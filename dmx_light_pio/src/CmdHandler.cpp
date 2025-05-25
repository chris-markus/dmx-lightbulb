// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "CmdHandler.h"

#include "command.h"
#include "InstanceManager.h"
#include "SettingsManager.h"
#include "driver.h"

Command commands[num_commands] = {
  cmd_networkControl,
  cmd_arduinoOTAEnable,
  cmd_color,
  cmd_dmxAddress,
  cmd_dmxUniverse,
  cmd_reboot,
  cmd_ssid,
  cmd_pasphrase,
  cmd_alias
};

bool boolCmdHelper(void* setting, const char* value) {
  Setting<bool> *_setting = (Setting<bool>*)(setting);
  if (strcmp(value, "1") == 0) {
    (*_setting).set(true);
  }
  else if (strcmp(value, "0") == 0){
    (*_setting).set(false);
  }
  else {
    return false;
  }
  return true;
}

bool numericCmdHelper(void* setting_v, const char* val_str, int highVal, int lowVal) {
  Setting<int> *_setting = (Setting<int>*)(setting_v);
  int val = strtol(val_str, NULL, 10);
  if (val <= highVal && val >= lowVal) {
    (*_setting).set(val);
    return true;
  }
  return false;
}

template <typename T>
bool stringCmdHelper(void* setting, const char* val_str) {
  if (setting == nullptr || val_str == nullptr || strlen(val_str) == 0)
  {
    return false;
  }
  Setting<T> *_setting = (Setting<T>*)(setting);
  T str = {};
  strncpy(str.data(), val_str, sizeof(T));
  _setting->set(str);
  return true;
}

bool cmd_DMXAddressHandler(void* setting, const char* value) {
  int high = DMX_MAX_ADDRESS; 
  int low = 1;
  return numericCmdHelper(setting, value, high, low);
}

bool cmd_DMXUniverseHandler(void* setting, const char* value) {
  int high = DMX_MAX_UNIVERSE;
  int low = 0;
  return numericCmdHelper(setting, value, high, low);
}

bool cmd_colorHandler(void* setting, const char* value) {
  String colorString(value);
  Settings::artnetEnable.set(false);
  Settings::sACNEnable.set(false);
  if (colorString.length() >= 8) {
    int start = 0;
    if(colorString.substring(0,1) == "#") {
      start = 1;
    }
    unsigned long colorVal = strtoll(colorString.substring(start).c_str(), NULL, 16);
    globals.led_interface.write_color(LedInterface::Priority::CONSOLE, 
                (uint8_t)((colorVal>>((unsigned long)(24)))), 
                (uint8_t)((colorVal&0xFF0000)>>16), 
                (uint8_t)((colorVal&0xFF00)>>8), 
                (uint8_t)(colorVal&0xFF));
    return true;
  }
  return false;
}

bool cmd_networkControlHandler(void* setting, const char* value) {
  if (strcmp(value, CMD_NETWORK_VALUE_ARTNET) == 0) {
    drv_artnet.setupFunc();
    Settings::artnetEnable.set(true);
    Settings::sACNEnable.set(false);
  }
  else if (strcmp(value, CMD_NETWORK_VALUE_SACN) == 0) {
    drv_sACN.setupFunc();
    Settings::artnetEnable.set(false);
    Settings::sACNEnable.set(true);
  }
  else if (strcmp(value, CMD_NETWORK_VALUE_NONE) == 0) {
    Settings::artnetEnable.set(false);
    Settings::sACNEnable.set(false);
  }
  else {
    return false;
  }
  return true;
}

bool cmd_arduinoOTAHandler(void* setting, const char* value) {
  bool retVal = boolCmdHelper(setting, value);
  if (retVal && Settings::arduinoOTAEnable.get()) {
    drv_OTA.setupFunc();
  }
  return retVal;
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
  (void*)(&Settings::arduinoOTAEnable),
  cmd_arduinoOTAHandler,
};

Command cmd_color = {
  CMD_COLOR_NAME,
  NULL,
  cmd_colorHandler,
};

Command cmd_dmxAddress = {
  CMD_DMXADDRESS_NAME,
  (void*)(&Settings::DMXAddress),
  cmd_DMXAddressHandler,
};

Command cmd_dmxUniverse = {
  CMD_DMXUNIVERSE_NAME,
  (void*)(&Settings::DMXUniverse),
  cmd_DMXUniverseHandler,
};

Command cmd_reboot = {
  CMD_REBOOT_NAME,
  NULL,
  cmd_rebootHandler,
};

Command cmd_ssid = {
  CMD_SSID_SET_NAME,
  (void*)(&Settings::ssid),
  stringCmdHelper<char_array_32>,
};

Command cmd_pasphrase = {
  CMD_SSID_SET_NAME,
  (void*)(&Settings::password),
  stringCmdHelper<char_array_32>,
};

Command cmd_alias = {
  CMD_ALIAS_SET_NAME,
  (void*)(&Settings::alias),
  stringCmdHelper<char_array_16>,
};