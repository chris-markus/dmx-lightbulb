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

extern Command commands[];

#endif