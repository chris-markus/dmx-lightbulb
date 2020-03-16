// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

// inspired by Tasmota settings strategy:
// https://github.com/arendst/Tasmota

#ifndef SETTINGS_H
#define SETTINGS_H

struct CONFIG {
    bool saveColor = true;
    int DMXUniverse = 1;
    int DMXAddress = 1;
    bool sACNEnable = false;
    bool artnetEnable = false;
    int sACNPollDelay = 16; // ms (fast enough to keep up with 60Hz)
    int artnetPollDelay = 25; // ms
    bool arduinoOTAEnable = true;
    bool staticIP = false;
    char ipAddress[16];
} Settings;

#endif