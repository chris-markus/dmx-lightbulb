// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "driver.h"
#include "settings.h"

#ifndef WHITE_PIN
    #define WHITE_PIN 12
#endif

#include <ArtnetWifi.h>

ArtnetWifi artnet;

void artnetSetup() {
    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
}

void artnetLoop() {
    artnet.read();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universe == Settings.DMXUniverse)
  {
    uint16_t address = Settings.DMXAddress - 1; // data is indexed from 0, address starts at 1
    if (address < )
    setLEDColor(data[address], 
                data[address + 1], 
                data[address + 2], 
                data[address + 3]);
  }
}

struct Driver drv_artnet() {
    return (Driver){
        &Settings.artnetPollDelay, 
        artnetSetup, 
        artnetLoop, 
        &Settings.artnetEnable
    };
}