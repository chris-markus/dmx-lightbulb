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
#include <ArduinoJson.h>

// Color temperature constants
const uint16_t COLOR_TEMP_NEUTRAL = 326;  // Neutral white temperature
const uint16_t COLOR_TEMP_MAX_DISTANCE = 174;  // Distance from neutral to either edge

struct ColorState {
    enum class Mode {
        RGB,
        COLOR_TEMP
    };

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint16_t color_temp = COLOR_TEMP_NEUTRAL;  // Default to neutral
    uint8_t brightness = 255;  // Default to full brightness
    bool is_on = true;  // Default to on
    Mode mode = Mode::COLOR_TEMP;  // Default to color temp mode

    void get_output_color(uint8_t& r_out, uint8_t& g_out, uint8_t& b_out, uint8_t& w_out) const {
        if (!is_on) {
            r_out = g_out = b_out = w_out = 0;
            return;
        }

        if (mode == Mode::RGB) {
            // In RGB mode, apply brightness to RGB values
            r_out = (r * brightness) / 255;
            g_out = (g * brightness) / 255;
            b_out = (b * brightness) / 255;
            w_out = 0;  // No white in RGB mode
        } else {
            // In color temp mode, calculate white level and add blue/red for temperature
            // Calculate how far we are from neutral as a percentage
            // 0% = at neutral, 100% = at edges
            uint16_t temp_distance = (abs(color_temp - COLOR_TEMP_NEUTRAL) * 100) / COLOR_TEMP_MAX_DISTANCE;
            
            // Reduce white level by up to 50% at the edges
            // temp_distance is 0-100, so divide by 4 to get 0-50
            uint16_t white_reduction = (temp_distance * brightness) / 200;  // (temp_distance/100 * brightness/4)
            w_out = brightness - white_reduction;  // Apply reduction to white level
            
            // Calculate temperature adjustment
            // Warmer than neutral -> more red, Cooler than neutral -> more blue
            if (color_temp > COLOR_TEMP_NEUTRAL) {  // Warmer than neutral
                // Add red for warmth
                r_out = ((color_temp - COLOR_TEMP_NEUTRAL) * brightness) / COLOR_TEMP_MAX_DISTANCE;  // Scale to brightness
                g_out = 0;
                b_out = 0;
            } else {  // Cooler than neutral
                // Add blue for coolness
                r_out = 0;
                g_out = 0;
                b_out = ((COLOR_TEMP_NEUTRAL - color_temp) * brightness) / COLOR_TEMP_MAX_DISTANCE;  // Scale to brightness
            }
        }
    }

    void clear() {
        r = 0;
        g = 0;
        b = 0;
        color_temp = COLOR_TEMP_NEUTRAL;
    }
};

const int mqtt_port = 1883;
const unsigned long STATUS_INTERVAL = 5000; // 5 seconds
const unsigned long SETUP_INTERVAL = 1000; // 1 second

WiFiClient wifi_client{};
PubSubClient client(wifi_client);

bool mqtt_initialized = false;
unsigned long last_status_publish = 0;
unsigned long last_setup_attempt = 0;  // Add timestamp for setup attempts
ColorState cached_color;

bool pending_response = false;

void publish_status() {
    if (!mqtt_initialized || !client.connected()) {
        return;
    }
    
    String alias = Settings::alias.get().data();
    String topic = "light/" + alias + "/status";
    
    // Create JSON status message
    JsonDocument doc;
    doc["state"] = cached_color.is_on ? "ON" : "OFF";
    
    if (cached_color.mode == ColorState::Mode::RGB) {
        JsonObject color = doc["color"].to<JsonObject>();
        color["r"] = cached_color.r;
        color["g"] = cached_color.g;
        color["b"] = cached_color.b;
    } else {
        doc["color_temp"] = cached_color.color_temp;
    }
    
    doc["brightness"] = cached_color.brightness;
    
    String output;
    serializeJson(doc, output);
    
    client.publish(topic.c_str(), output.c_str());
}

void publish_discovery() {
    if (!mqtt_initialized || !client.connected()) {
        return;
    }
    
    // Reference: https://resinchemtech.blogspot.com/2023/12/mqtt-auto-discovery.html
    String alias = Settings::alias.get().data();
    String topic = "homeassistant/light/" + alias + "/config";

    String name = alias;
    name.replace("_", " ");

    // Create JSON discovery message
    JsonDocument doc;

    //Create JSON payload per HA documentation
    doc["name"] = name;
    doc["uniq_id"] = alias;
    doc["stat_t"] = "light/" + alias + "/status";
    doc["cmd_t"] = "light/" + alias + "/cmd";
    doc["schema"] = "json";
    doc["brightness"] = true;
    doc["platform"] = "light";
    JsonArray supported_color_modes = doc["supported_color_modes"].to<JsonArray>();
    supported_color_modes.add("rgb");
    supported_color_modes.add("color_temp");
    JsonObject device = doc["device"].to<JsonObject>();
    device["ids"] = alias + "dev";
    device["name"] = name;
    device["mf"] = "Chris Markus";
    device["mdl"] = "ESP8266";
    device["sw"] = "1.0";
    device["hw"] = "1.0";
    device["cu"] = "http://" + WiFi.localIP().toString() + "/";
    doc["qos"] = 2;
    
    String output;
    serializeJson(doc, output);
    
    const bool result = client.publish(topic.c_str(), output.c_str(), true);

    Serial.printf("Published discovery message, result: %d\n", result);
}

void on_data(char *topic, byte *payload, unsigned int length)
{
    String topic_str(topic);
    String alias = Settings::alias.get().data();

    if (topic_str == "homeassistant/status") {
        Serial.println("MQTT Birth Message received");
        publish_discovery();
        return;
    }
    
    // Check if this is a message for our device and ends with /cmd
    if (!topic_str.startsWith("light/" + alias + "/cmd")) {
        return;
    }

    // Queue a response to be sent.
    pending_response = true;
    
    // Parse JSON payload
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Handle state
    if (doc["state"].is<const char*>()) {
        cached_color.is_on = (strcmp(doc["state"], "ON") == 0);
    }


    const bool has_color = doc["color"].is<JsonObject>();
    const bool has_color_temp = doc["color_temp"].is<int>();

    if (has_color || has_color_temp) {
        cached_color.clear();
    }

    // Handle brightness
    if (doc["brightness"].is<int>()) {
        cached_color.brightness = doc["brightness"];
    }

    // Handle RGB color
    if (has_color) {
        JsonObject color = doc["color"].as<JsonObject>();
        if (color["r"].is<int>()) cached_color.r = color["r"] | 0;
        if (color["g"].is<int>()) cached_color.g = color["g"] | 0;
        if (color["b"].is<int>()) cached_color.b = color["b"] | 0;
        cached_color.mode = ColorState::Mode::RGB;
    }
    // Handle color temperature
    else if (has_color_temp) {
        cached_color.color_temp = doc["color_temp"];
        cached_color.mode = ColorState::Mode::COLOR_TEMP;
    }

    // Write final color
    uint8_t r, g, b, w;
    cached_color.get_output_color(r, g, b, w);
    globals.led_interface.write_color(LedInterface::Priority::MQTT, r, g, b, w);
}

void mqttSetup() {
    char buf[sizeof(Settings::mqtt_broker) + 1];
    strncpy(buf, Settings::mqtt_broker.get().data(), sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    client.setServer(buf, mqtt_port);
    client.setCallback(on_data);
    client.setBufferSize(512);
    
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
        String topic = "light/" + alias + "/cmd";
        client.subscribe(topic.c_str());
        client.subscribe("homeassistant/status");
        mqtt_initialized = true;
        publish_discovery();
    } else {
        Serial.println("MQTT Connection failed!");
        mqtt_initialized = false;
    }
}

void mqttLoop() {
    if (mqtt_initialized) {
        client.loop();
        
        // Publish status every second or if a response is pending.
        unsigned long now = millis();
        if (now - last_status_publish >= STATUS_INTERVAL || pending_response) {
            publish_status();
            last_status_publish = now;
            pending_response = false;
        }
    }
    else {
        // Only attempt setup every second.
        const unsigned long now = millis();
        if (now - last_setup_attempt >= SETUP_INTERVAL) {
            Serial.println("MQTT Disconnected, attempting to reconnect...");
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