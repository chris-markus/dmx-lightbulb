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

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include "settings.h"
#include "driver.h"
#include "console.h"

#ifndef STASSID
#define STASSID "Bulbs"
#define STAPSK  "lightbulb"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");
  LEDSetup();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  driverSetup();

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  strcpy(Settings.ipAddress, WiFi.localIP().toString().c_str());
}

void loop() {
  driverLoop();
  // for background tasks like wifi
  delay(1);
}
