// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#pragma once

#include <Arduino.h>
#include <functional>

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

// reboot command
#define CMD_REBOOT_NAME             "reboot"

// ssid command
#define CMD_SSID_SET_NAME           "ssid"

// password command
#define CMD_PASSWORD_SET_NAME       "passphrase"

// alias command
#define CMD_ALIAS_SET_NAME          "alias"

// ---------------------------------------------------------------------------

typedef bool (*CommandFunc)(void*, const char*);

// TODO: make this typesafe
struct Command {
    const char* cmdName;
    void* setting;
    CommandFunc runCommand;
};

// defined in command_handler.cpp
extern Command cmd_networkControl;
extern Command cmd_arduinoOTAEnable;
extern Command cmd_color;
extern Command cmd_dmxAddress;
extern Command cmd_dmxUniverse;
extern Command cmd_reboot;
extern Command cmd_ssid;
extern Command cmd_pasphrase;
extern Command cmd_alias;

constexpr size_t num_commands = 10;

extern Command commands[num_commands];
