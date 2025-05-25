// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#pragma once

#include <Arduino.h>

class ConsoleClass : public Print {
public:
    inline ConsoleClass() {
        reportToWeb = false;
    }

    virtual size_t write(uint8_t);
    virtual size_t write(const char *);
private:
    bool reportToWeb;
};

extern ConsoleClass Console;
