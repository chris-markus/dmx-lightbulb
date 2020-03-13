// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================


#include <ESPAsyncE131.h>

#define UNIVERSE_COUNT 1

ESPAsyncE131 e131(UNIVERSE_COUNT);

#include "driver.h"
#include "settings.h"

void sACNSetup() {
  if (e131.begin(E131_MULTICAST, Settings.DMXUniverse, UNIVERSE_COUNT))   // Listen via Multicast
    Serial.println(F("sACN Listening"));
  else 
    Serial.println(F("sACN Init FAILED"));
}

void sACNLoop() {
  if (!e131.isEmpty()) {
    e131_packet_t packet;
    e131.pull(&packet);     // Pull packet from ring buffer
    if (packet.universe == Settings.DMXUniverse) {
      uint16_t address = Settings.DMXAddress;
      setLEDColor(packet.property_values[address], 
                  packet.property_values[address + 1], 
                  packet.property_values[address + 2], 
                  packet.property_values[address + 3]);
    }
  }
}

struct Driver drv_sACN() {
  return (Driver){
    &Settings.sACNPollDelay, 
    sACNSetup, 
    sACNLoop, 
    &Settings.sACNEnable
  };
}