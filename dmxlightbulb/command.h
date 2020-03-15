// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>

typedef bool (*CommandFunc)(void*, const char*);

struct Command {
    const char* cmdName;
    void* setting;
    CommandFunc runCommand;
};

extern Command cmd_networkControl;
extern Command cmd_arduinoOTAEnable;
extern Command cmd_color;
extern Command cmd_dmxAddress;
extern Command cmd_dmxUniverse;

Command commands[] = {
  cmd_networkControl,
  cmd_arduinoOTAEnable,
  cmd_color,
  cmd_dmxAddress,
  cmd_dmxUniverse
};

const int num_commands = sizeof(commands)/sizeof(Command);

#endif