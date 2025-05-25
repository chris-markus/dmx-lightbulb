// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

/* 
Required Libraries:
  - ArtnetWifi (Arduino Pkg Manager)
  - ESPAsyncE131 (Arduino Pkg Manager)
      - Requires ESPAsyncUDP: https://github.com/me-no-dev/ESPAsyncUDP
*/

#include <Arduino.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include "SettingsManager.h"
#include "driver.h"
#include "console.h"
#include "LedInterface.h"
#include "InstanceManager.h"

#ifndef STASSID
#define STASSID "Bulbs"
#define STAPSK  "lightbulb"
#endif

void LEDSetup(void);

const char* ssid = STASSID;
const char* password = STAPSK;

const char* mesh_ssid = "Bulbs";
const char* mesh_password = "lightbulb";

const uint8_t GPIO_SM16716_CLK = 5;
const uint8_t GPIO_SM16716_DAT = 4;
const uint8_t GPIO_SM16716_SEL = 13;
const uint8_t GPIO_WHITE_PWM   = 12;
const unsigned long WIFI_TIMEOUT = 10 * 1000;

void onWifiConnectPrimary();
void onWifiConnectBootstrap();
void onWifiDisconnect();

LedInterface led_interface {GPIO_SM16716_CLK, GPIO_SM16716_DAT, GPIO_SM16716_SEL, GPIO_WHITE_PWM};
WifiStateMachine wifi_mgr {onWifiConnectBootstrap, onWifiConnectPrimary, onWifiDisconnect};

/*
 * Some Globals... TODO: make these not globals
 */
InstanceManager globals(led_interface, wifi_mgr);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");
  
  led_interface.init();

  SettingsManager::initialize();

  wifi_mgr.initialize();

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  Serial.println("Ready");
  //strcpy(Settings::ipAddress, WiFi.localIP().toString().c_str());
}

void loop() {
  wifi_mgr.tick();
  if (wifi_mgr.connected()) {
    driverLoop();
  }
  // for background tasks like wifi
  delay(1);
}

void onWifiConnectPrimary() {
  Serial.print("IP address: ");
  Serial.println(globals.wifi_mgr.ip_address());
  driverSetup();
}

void onWifiConnectBootstrap() {
  Serial.print("IP address: ");
  Serial.println(globals.wifi_mgr.ip_address());
}

void onWifiDisconnect() {
  
}
