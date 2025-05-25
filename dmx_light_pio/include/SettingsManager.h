#pragma once

#include <Arduino.h>

using char_array_16 = std::array<char, 16>;
using char_array_32 = std::array<char, 32>;

struct Settings_t {
    uint16_t revision;
    int DMXUniverse;
    int DMXAddress;
    bool sACNEnable;
    bool artnetEnable;
    bool arduinoOTAEnable;
    char_array_32 ssid;
    char_array_32 password;
    char_array_16 alias;
    char_array_32 mqtt_broker;
    char_array_32 mqtt_username;
    char_array_32 mqtt_password;
};

// change when the Settings_t struct changes
static constexpr uint16_t settings_revision = 7;

//#define AddSetting(type, name) static Setting<type, offsetof(Settings_t, name)> name

class SettingsManager
{
public:
    static void initialize();
    static void write_to_eeprom();

private:
    static void serialize(Settings_t &s);
    static bool deserialize(const Settings_t &s);
    static void read_from_eeprom();
};


template <typename T>
struct Setting {
private:
    T value;
public:
    explicit Setting(T v) : value(v){}

    static constexpr size_t size() {
        return sizeof(T);
    }

    void set(const T val)
    {
        value = val;
        SettingsManager::write_to_eeprom();
    }

    void set_no_write(T val)
    {
        value = val;
    }

    T get() const
    {
        return value;
    }
};

namespace Settings
{
    extern Setting<int> DMXUniverse;
    extern Setting<int> DMXAddress;
    extern Setting<bool> sACNEnable;
    extern Setting<bool> artnetEnable;
    extern Setting<bool> arduinoOTAEnable;
    extern Setting<char_array_32> ssid;
    extern Setting<char_array_32> password;
    extern Setting<char_array_16> alias;
    extern Setting<char_array_32> mqtt_broker;
    extern Setting<char_array_32> mqtt_username;
    extern Setting<char_array_32> mqtt_password;
};
