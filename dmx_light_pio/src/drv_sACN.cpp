// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include <ESPAsyncE131.h>
#include "InstanceManager.h"

#define UNIVERSE_COUNT 1

ESPAsyncE131 e131(UNIVERSE_COUNT);

#include "driver.h"
#include "SettingsManager.h"

void sACNSetup() {
  static bool initialized = false;
  if (!initialized) {
    if (e131.begin(E131_MULTICAST, Settings::DMXUniverse.get(), UNIVERSE_COUNT))   // Listen via Multicast
      Serial.println(F("sACN Listening"));
    else 
      Serial.println(F("sACN Init FAILED"));
    initialized = true;
  }
}

void sACNLoop() {
  if (!e131.isEmpty()) {
    e131_packet_t packet;
    e131.pull(&packet);     // Pull packet from ring buffer
    if (packet.universe == Settings::DMXUniverse.get()) {
      uint16_t address = Settings::DMXAddress.get();
      Serial.printf("%d: %d\n", address, packet.property_values[address]);
      globals.led_interface.write_color(LedInterface::Priority::SACN, 
                  packet.property_values[address], 
                  packet.property_values[address + 1], 
                  packet.property_values[address + 2], 
                  packet.property_values[address + 3]);
    }
  }
}

void sACNCleanup() {
  
}

static inline bool get_enabled() {
  return Settings::sACNEnable.get();
}

static inline int get_poll_delay() {
  return 25; // ms
}

struct Driver drv_sACN = {
  get_poll_delay, 
  sACNSetup, 
  sACNLoop, 
  sACNCleanup,
  get_enabled
};