#pragma once

#include <Arduino.h>

#include <stdint.h>


class LedInterface {
public:
    enum class Priority {
        CONSOLE,  // Highest priority - direct console commands
        SACN,     // Streaming ACN
        ARTNET,   // ArtNet
        MQTT      // Lowest priority - MQTT commands
    };

    LedInterface(uint8_t pin_clk, uint8_t pin_dat, uint8_t pin_sel, uint8_t pin_pwm_w)
        : m_pin_clk(pin_clk), m_pin_dat(pin_dat), m_pin_sel(pin_sel), m_pin_pwm_w(pin_pwm_w)
    {
        // Initialize all colors to 0
        for (int i = 0; i < static_cast<int>(Priority::MQTT) + 1; i++) {
            m_colors[i][0] = 0;
            m_colors[i][1] = 0;
            m_colors[i][2] = 0;
            m_colors[i][3] = 0;
            m_last_update[i] = 0;
        }
    }

    void init();
    
    // Single entry point for all color updates
    void write_color(Priority priority, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    
    // Get current color values
    void get_current_color(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w) const {
        r = m_current_color[0];
        g = m_current_color[1];
        b = m_current_color[2];
        w = m_current_color[3];
    }

private:
    void write_color_internal(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void SM16716_Update(uint8_t duty_r, uint8_t duty_g, uint8_t duty_b);
    void SM16716_Init(void);

    inline void SM16716_SendBit(uint8_t v)
    {
        /* NOTE:
        * According to the spec sheet, max freq is 30 MHz, that is 16.6 ns per high/low half of the
        * clk square wave. That is less than the overhead of 'digitalWrite' at this clock rate,
        * so no additional delays are needed yet. */

        digitalWrite(m_pin_dat, (v != 0) ? HIGH : LOW);
        digitalWrite(m_pin_clk, HIGH);
        digitalWrite(m_pin_clk, LOW);
    }

    inline void SM16716_SendByte(uint8_t v)
    {
        for (uint8_t mask = 0x80; mask; mask >>= 1) {
            SM16716_SendBit(v & mask);
        }
    }

    const uint8_t m_pin_clk;
    const uint8_t m_pin_dat;
    const uint8_t m_pin_sel;
    const uint8_t m_pin_pwm_w;
    
    uint8_t m_colors[4][4];  // [priority][r,g,b,w]
    uint8_t m_current_color[4];  // Current active color
    unsigned long m_last_update[4]; // One for each priority level
    
    // Timeouts for each priority level (in milliseconds)
    static constexpr unsigned long TIMEOUTS[4] = {
        0,      // CONSOLE: never timeout
        60000,  // SACN: 1 minute
        60000,  // ARTNET: 1 minute
        0       // MQTT: never timeout
    };
};
