// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

// --------------------------- Command Constants -----------------------------

// redirect parameter
#define REDIRECT                    "redirect"

// network control command
#define CMD_NETWORK_NAME            "networkControl"
#define CMD_NETWORK_VALUE_ARTNET    "artnet"
#define CMD_NETWORK_VALUE_SACN      "sacn"
#define CMD_NETWORK_VALUE_NONE      "none"

// arduino OTA command
#define CMD_ARDUINOOTA_NAME         "arduinoOTAEnable"
#define CMD_ARDUINOOTA_VALUE_TRUE   "1"
#define CMD_ARDUINOOTA_VALUE_FALSE  "0"

// color command
#define CMD_COLOR_NAME              "color"

// dmx address command
#define CMD_DMXADDRESS_NAME         "dmxAddress"
#define DMX_MAX_ADDRESS             512

// dmx universe command
#define CMD_DMXUNIVERSE_NAME        "dmxUniverse"
#define DMX_MAX_UNIVERSE            128

// save color command
#define CMD_SAVECOLOR_NAME          "saveColor"

// reboot command
#define CMD_REBOOT_NAME             "reboot"

// ---------------------------------------------------------------------------

typedef bool (*CommandFunc)(void*, const char*);

struct Command {
    const char* cmdName;
    void* setting;
    CommandFunc runCommand;
};

// defined in command_handler.ino:
extern Command cmd_networkControl;
extern Command cmd_arduinoOTAEnable;
extern Command cmd_color;
extern Command cmd_dmxAddress;
extern Command cmd_dmxUniverse;
extern Command cmd_saveColor;
extern Command cmd_reboot;

Command commands[] = {
  cmd_networkControl,
  cmd_arduinoOTAEnable,
  cmd_color,
  cmd_dmxAddress,
  cmd_dmxUniverse,
  cmd_saveColor,
  cmd_reboot,
};

const int num_commands = sizeof(commands)/sizeof(Command);

#endif