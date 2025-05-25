// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "driver.h"
#include "SettingsManager.h"
#include "InstanceManager.h"

#include <ArtnetWifi.h>

ArtnetWifi artnet;

bool artnetInitialized = false;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);

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
      //Serial.print("Avg time per tick:");
      //Serial.println((millis() - lastUpdate)/numUpdates);
      lastUpdate = millis();
      numUpdates = 0;
    }
    numUpdates++;
    artnet.read();
  }
}

void artnetCleanup() {
  
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universe == Settings::DMXUniverse.get())
  {
    uint16_t address = Settings::DMXAddress.get() - 1; // data is indexed from 0, address starts at 1
    //if (address < DMX_MAX_UNIVERSE)
    //Serial.printf("%d: %d\n", address, data[address]);
    globals.led_interface.write_color(LedInterface::Priority::ARTNET, data[address],
                data[address + 1],
                data[address + 2],
                data[address + 3]);
  }
}

static inline bool get_enabled() {
  return Settings::artnetEnable.get();
}

static inline int get_poll_delay() {
  return 25; // ms
}

struct Driver drv_artnet = {
  get_poll_delay,
  artnetSetup,
  artnetLoop,
  artnetCleanup,
  get_enabled
};