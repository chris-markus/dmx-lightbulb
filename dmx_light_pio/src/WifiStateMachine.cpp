
#include "WiFiStateMachine.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "WebServer.h"
#include <ESP8266mDNS.h>


static const char *bootstrap_ssid = "Bulbs";
static const char *bootstrap_password = "l!ghtbu1b";

static constexpr unsigned long scan_timeout = 10 * 1000;
static constexpr unsigned long bootstrap_timeout = 30 * 1000;

#define PrintIfNot(condition) \
    if (!(condition)) { \
        Serial.printf("Failed: " __FILE__ ":%d: " #condition "\n", __LINE__); \
    }

void WifiStateMachine::initialize()
{
    last_state_change = millis();
    state = WifiState::ScanningPrimary;
    last_state = WifiState::COUNT;
}

const String WifiStateMachine::ip_address()
{
    return WiFi.localIP().toString();
}

void WifiStateMachine::tick()
{
    switch(state) {
        case WifiState::ScanningPrimary:
            if (last_state != WifiState::ScanningPrimary) {
                Serial.println("Scanning for primary network...");
                WiFi.mode(WIFI_STA);
                WiFi.begin(Settings::ssid.get().data(), Settings::password.get().data());
                last_state = WifiState::ScanningPrimary;
                last_state_change = millis();
            }

            if (WiFi.status() == WL_CONNECTED) {
                state = WifiState::ConnectedPrimary;
                last_state_change = millis();
            }
            else if (millis() - last_state_change > scan_timeout) {
                state = WifiState::BootstrapAP;
                last_state_change = millis();
            }
            break;
        case WifiState::ConnectedPrimary:
            if (last_state != WifiState::ConnectedPrimary) {
                Serial.println("Connected to primary network!");
                is_connected = true;
                cb_connect_primary();
                last_state = WifiState::ConnectedPrimary;
            }

            if (!WiFi.isConnected()) {
                is_connected = false;
                cb_disconnect();
                state = WifiState::ScanningPrimary;
                last_state_change = millis();
            }
            break;
        case WifiState::BootstrapAP:
            // TODO: check if the bootstrap AP is already present
            if (last_state != WifiState::BootstrapAP) {
                Serial.println("No networks found! Starting AP...");
                PrintIfNot(WiFi.mode(WIFI_AP));
                PrintIfNot(WiFi.softAPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0)));

                PrintIfNot(WiFi.softAP(bootstrap_ssid, bootstrap_password));

                cb_connect_bootstrap();
                
                StartWebserver();

                last_state = WifiState::BootstrapAP;
            }

            if (WiFi.softAPgetStationNum() > 0) {
                WebserverLoop();
                delay(50);
            }
            else if (millis() - last_state_change > bootstrap_timeout) {
                StopWebserver();
                WiFi.softAPdisconnect();
                cb_disconnect();
                state = WifiState::ScanningPrimary;
                last_state_change = millis();
            }
            break;
        default:
            // TODO: log error
            state = WifiState::ScanningPrimary;
            break;
    }
}