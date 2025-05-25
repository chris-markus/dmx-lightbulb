// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#pragma once

#include <Arduino.h>
#include <functional>

typedef void (*DriverFunc)(void);

struct Driver {
  int (*pollInterval)(void);
  DriverFunc setupFunc;
  DriverFunc loopFunc;
  DriverFunc cleanupFunc;
  bool (*is_enabled)(void);
};

extern Driver drv_sACN;
extern Driver drv_webServer;
extern Driver drv_OTA;
extern Driver drv_artnet;
extern Driver drv_mqtt;

void driverSetup();
void driverLoop();
