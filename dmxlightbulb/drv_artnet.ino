// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "driver.h"
#include "settings.h"

#include <ArtnetWifi.h>

ArtnetWifi artnet;

bool artnetInitialized = false;

void artnetSetup() {
  if (!artnetInitialized) {
    artnet.setArtDmxCallback(onDmxFrame);
    artnet.begin();
    artnetInitialized = true;
  }
}

void artnetLoop() {
  if (artnetInitialized) {
    static unsigned long lastUpdate = 0;
    static int numUpdates = 0;
    if (millis() > lastUpdate + 1000) {
      Serial.print("Avg time per tick:");
      Serial.println((millis() - lastUpdate)/numUpdates);
      lastUpdate = millis();
      numUpdates = 0;
    }
    numUpdates++;
    artnet.read();
  }
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universe == Settings.DMXUniverse)
  {
    uint16_t address = Settings.DMXAddress - 1; // data is indexed from 0, address starts at 1
    //if (address < DMX_MAX_UNIVERSE)
    setLEDColor(data[address], 
                data[address + 1], 
                data[address + 2], 
                data[address + 3]);
  }
}

struct Driver drv_artnet = {
  &Settings.artnetPollDelay, 
  artnetSetup, 
  artnetLoop, 
  &Settings.artnetEnable
};