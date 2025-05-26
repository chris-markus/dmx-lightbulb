// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "driver.h"
#include "SettingsManager.h"
#include "InstanceManager.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int mqtt_port = 1883;
const unsigned long STATUS_INTERVAL = 1000; // 1 second
const unsigned long SETUP_INTERVAL = 1000; // 1 second

WiFiClient wifi_client{};
PubSubClient client(wifi_client);

bool mqtt_initialized = false;
unsigned long last_status_publish = 0;
unsigned long last_setup_attempt = 0;  // Add timestamp for setup attempts
uint8_t cached_color[4] = {0, 0, 0, 0}; // Store last color when power is turned off

void publish_status() {
    if (!mqtt_initialized || !client.connected()) {
        return;
    }
    
    String alias = Settings::alias.get().data();
    String topic = "light/" + alias + "/status";
    
    // Get current color values
    uint8_t r, g, b, w;
    globals.led_interface.get_current_color(r, g, b, w);
    
    // Create binary payload [R,G,B,W]
    uint8_t payload[4] = {r, g, b, w};
             
    client.publish(topic.c_str(), payload, 4);
}

void on_color(char *data, unsigned int len)
{
    if (len != 4) {
        Serial.println("Invalid color packet - expected 4 bytes");
        return;
    }
    
    // Cache the color
    memcpy(cached_color, data, 4);
    
    // Data format: [R, G, B, W]
    globals.led_interface.write_color(LedInterface::Priority::MQTT, data[0], data[1], data[2], data[3]);
}

void on_power(char *data, unsigned int len)
{
    if (len != 1) {
        Serial.println("Invalid power packet - expected 1 byte");
        return;
    }
    
    // Data format: [0 or 1]
    if (data[0] == 0) {
        // Turn off - store current color before turning off
        uint8_t r, g, b, w;
        globals.led_interface.get_current_color(r, g, b, w);
        cached_color[0] = r;
        cached_color[1] = g;
        cached_color[2] = b;
        cached_color[3] = w;
        
        // Turn off
        globals.led_interface.write_color(LedInterface::Priority::MQTT, 0, 0, 0, 0);
    } else {
        // Turn on - restore last color
        globals.led_interface.write_color(LedInterface::Priority::MQTT, cached_color[0], cached_color[1], cached_color[2], cached_color[3]);
    }
}

void on_data(char *topic, byte *payload, unsigned int length)
{
    String topic_str(topic);
    String alias = Settings::alias.get().data();
    
    // Check if this is a message for our device
    if (!topic_str.startsWith("light/" + alias + "/")) {
        return;
    }
    
    // Extract the command part of the topic
    String command = topic_str.substring(("light/" + alias + "/").length());
    
    if (command == "color") {
        on_color((char*)payload, length);
    }
    else if (command == "on") {
        on_power((char*)payload, length);
    }
}

void mqttSetup() {
    char buf[sizeof(Settings::mqtt_broker) + 1];
    strncpy(buf, Settings::mqtt_broker.get().data(), sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    client.setServer(buf, mqtt_port);
    client.setCallback(on_data);
    
    String alias = Settings::alias.get().data();
    String client_id = "lightbulb_" + alias;
    
    Serial.printf("Connecting to %s with username %s and password %s.\n", 
                 buf, 
                 Settings::mqtt_username.get().data(), 
                 Settings::mqtt_password.get().data());
                 
    bool res = client.connect(client_id.c_str(), 
                            Settings::mqtt_username.get().data(), 
                            Settings::mqtt_password.get().data());
                            
    if (res) {
        Serial.println("MQTT Connected!");
        String topic = "light/" + alias + "/#";
        client.subscribe(topic.c_str());
        mqtt_initialized = true;
    } else {
        Serial.println("MQTT Connection failed!");
        mqtt_initialized = false;
    }
}

void mqttLoop() {
    if (mqtt_initialized) {
        client.loop();
        
        // Publish status every second
        unsigned long now = millis();
        if (now - last_status_publish >= STATUS_INTERVAL) {
            publish_status();
            last_status_publish = now;
        }
    }
    else {
        // Only attempt setup every second
        const unsigned long now = millis();
        if (now - last_setup_attempt >= SETUP_INTERVAL) {
            mqttSetup();
            last_setup_attempt = now;
        }
    }
}

void mqttCleanup() {
    client.disconnect();
    mqtt_initialized = false;
}

static inline bool get_enabled() {
    return true; // Always enabled for now
}

static inline int get_poll_delay() {
    return 10;
}

struct Driver drv_mqtt = {
    get_poll_delay,
    mqttSetup,
    mqttLoop,
    mqttCleanup,
    get_enabled
};