// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "console.h"

size_t ConsoleClass::write(uint8_t c) {
    Serial.write(c);
    return 0;
}

size_t ConsoleClass::write(const char *str) {
    Serial.write(str);
    return 0;
}

ConsoleClass Console;