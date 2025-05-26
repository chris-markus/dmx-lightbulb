#include "SettingsManager.h"

#include <EEPROM.h>

static constexpr size_t settings_size = sizeof(Settings_t);

using crc_t = unsigned long;
static constexpr int crc_addr = 0;
static constexpr int settings_addr = sizeof(crc_t);


static constexpr crc_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

static constexpr Settings_t default_settings = {
    .revision = settings_revision,
    .DMXUniverse = 0,
    .DMXAddress = 1,
    .sACNEnable = false,
    .artnetEnable = true,
    .arduinoOTAEnable = true,
    .ssid = {{""}},
    .password = {{""}},
    .alias = {{"light"}},
    .mqtt_broker = {{""}},
    .mqtt_username = {{""}},
    .mqtt_password = {{""}}
};

static crc_t compute_settings_crc(const Settings_t *settings)
{
    crc_t crc = ~0L;
    uint8_t *buf = (uint8_t *)settings;
    for (size_t index = 0 ; index < settings_size; ++index)
    {
        crc = crc_table[(crc ^ buf[index]) & 0x0f] ^ (crc >> 4);
        crc = crc_table[(crc ^ (buf[index] >> 4)) & 0x0f] ^ (crc >> 4);
        crc = ~crc;
    }
    return crc;
}

void SettingsManager::initialize()
{
    EEPROM.begin(sizeof(Settings_t) + sizeof(crc_t));
    read_from_eeprom();
}

void SettingsManager::serialize(Settings_t &s)
{
    s.revision = settings_revision;
    s.DMXUniverse = Settings::DMXUniverse.get();
    s.DMXAddress = Settings::DMXAddress.get();
    s.sACNEnable = Settings::sACNEnable.get();
    s.artnetEnable = Settings::artnetEnable.get();
    s.arduinoOTAEnable = Settings::arduinoOTAEnable.get();
    s.ssid = Settings::ssid.get();
    s.password = Settings::password.get();
    s.alias = Settings::alias.get();
    s.mqtt_broker = Settings::mqtt_broker.get();
    s.mqtt_username = Settings::mqtt_username.get();
    s.mqtt_password = Settings::mqtt_password.get();
}

bool SettingsManager::deserialize(const Settings_t &s)
{
    if (s.revision == settings_revision) {
        Settings::DMXUniverse.set_no_write(s.DMXUniverse);
        Settings::DMXAddress.set_no_write(s.DMXAddress);
        Settings::sACNEnable.set_no_write(s.sACNEnable);
        Settings::artnetEnable.set_no_write(s.artnetEnable);
        Settings::arduinoOTAEnable.set_no_write(s.arduinoOTAEnable);
        Settings::ssid.set_no_write(s.ssid);
        Settings::password.set_no_write(s.password);
        Settings::alias.set_no_write(s.alias);
        Settings::mqtt_broker.set_no_write(s.mqtt_broker);
        Settings::mqtt_username.set_no_write(s.mqtt_username);
        Settings::mqtt_password.set_no_write(s.mqtt_password);
        return true;
    }
    else {
        return false;
    }
}

void SettingsManager::write_to_eeprom()
{
    Settings_t s;
    Serial.println("writing to EEPROM.");
    SettingsManager::serialize(s);
    crc_t crc = compute_settings_crc(&s);
    EEPROM.put(settings_addr, s);
    EEPROM.put(crc_addr, crc);
    EEPROM.commit();
}

void SettingsManager::read_from_eeprom()
{
    bool valid = false;

    Settings_t temp_settings;
    
    crc_t crc_read;
    EEPROM.get(crc_addr, crc_read);
    EEPROM.get(settings_addr, temp_settings);
    crc_t crc_computed = compute_settings_crc(&temp_settings);

    if (crc_computed == crc_read)
    {
        SettingsManager::deserialize(temp_settings);
        valid = true;
    }
    
    if (!valid)
    {
        Serial.println("Eeprom settings invalid, loading defaults.");
    }
}

namespace Settings
{
    Setting<int> DMXUniverse {default_settings.DMXUniverse};
    Setting<int> DMXAddress {default_settings.DMXAddress};
    Setting<bool> sACNEnable {default_settings.sACNEnable};
    Setting<bool> artnetEnable {default_settings.artnetEnable};
    Setting<bool> arduinoOTAEnable {default_settings.arduinoOTAEnable};
    Setting<char_array_32> ssid {default_settings.ssid};
    Setting<char_array_32> password {default_settings.password};
    Setting<char_array_16> alias {default_settings.alias};
    Setting<char_array_32> mqtt_broker {default_settings.mqtt_broker};
    Setting<char_array_32> mqtt_username {default_settings.mqtt_username};
    Setting<char_array_32> mqtt_password {default_settings.mqtt_password};
}
