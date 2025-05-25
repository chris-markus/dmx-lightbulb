// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================
#include <ArduinoOTA.h>

#include "driver.h"
#include "SettingsManager.h"

constexpr int OTADelay = 50;

void OTASetup() {
  static bool initialized = false;
  if (!initialized) {
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }
      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
    initialized = false;
  }
}

void OTACleanup() {
  
}

void OTALoop() {
  ArduinoOTA.handle();
}

static inline bool get_enabled() {
  return Settings::arduinoOTAEnable.get();
}

static inline int get_poll_delay() {
  return OTADelay;
}

struct Driver drv_OTA = {
  get_poll_delay, // ms
  OTASetup,
  OTALoop,
  OTACleanup,
  get_enabled
};