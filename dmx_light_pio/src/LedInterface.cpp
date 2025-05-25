#include "LedInterface.h"

void LedInterface::SM16716_Update(uint8_t duty_r, uint8_t duty_g, uint8_t duty_b)
{
    /*
    if (Sm16716.pin_sel < 99) {
        bool should_enable = (duty_r | duty_g | duty_b);
        if (!Sm16716.enabled && should_enable) {
        //DEBUG_DRIVER_LOG(PSTR(D_LOG_SM16716 "turning color on"));
        Sm16716.enabled = true;
        digitalWrite(Sm16716.pin_sel, HIGH);
        // in testing I found it takes a minimum of ~380us to wake up the chip
        // tested on a Merkury RGBW with an SM726EB
        delayMicroseconds(1000);
        SM16716_Init();
        }
        else if (Sm16716.enabled && !should_enable) {
        Sm16716.enabled = false;
        }
    }*/

    // send start bit
    SM16716_SendBit(1);
    SM16716_SendByte(duty_r);
    SM16716_SendByte(duty_g);
    SM16716_SendByte(duty_b);

    // send a 'do it' pulse
    // (if multiple chips are chained, each one processes the 1st '1rgb' 25-bit block and
    // passes on the rest, right until the one starting with 0)
    SM16716_SendBit(0);
    SM16716_SendByte(0);
    SM16716_SendByte(0);
    SM16716_SendByte(0);
}

void LedInterface::SM16716_Init()
{
    // Enable the chip.
    digitalWrite(m_pin_sel, HIGH);
    delayMicroseconds(1000);

    for (uint32_t i = 0; i < 50; ++i) {
        SM16716_SendBit(0);
    }

    // Do our best to prevent the leds from flashing on init.
    unsigned long start = millis();
    while (millis() < start + 2) {
      SM16716_Update(0,0,0);
    }
}

void LedInterface::init()
{
    // init pins
    pinMode(m_pin_clk, OUTPUT);
    digitalWrite(m_pin_clk, LOW);

    pinMode(m_pin_dat, OUTPUT);
    digitalWrite(m_pin_dat, LOW);

    pinMode(m_pin_sel, OUTPUT);
    digitalWrite(m_pin_sel, LOW);

    pinMode(m_pin_pwm_w, OUTPUT);
    digitalWrite(m_pin_pwm_w, LOW);

    // default to lightbulb on!
    analogWrite(m_pin_pwm_w, 255);

    // init SM16716
    SM16716_Init();
}

void LedInterface::write_color_internal(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    SM16716_Update(r, g, b);
    analogWrite(m_pin_pwm_w, w);
}

void LedInterface::write_color(Priority priority, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    // Store the color for this priority level
    int pri = static_cast<int>(priority);
    m_colors[pri][0] = r;
    m_colors[pri][1] = g;
    m_colors[pri][2] = b;
    m_colors[pri][3] = w;
    m_last_update[pri] = millis();
    
    // Check each priority level from highest to lowest
    unsigned long now = millis();
    bool updated = false;
    
    for (int i = 0; i <= static_cast<int>(Priority::MQTT); i++) {
        if (m_last_update[i] > 0 && (TIMEOUTS[i] == 0 || (now - m_last_update[i] <= TIMEOUTS[i]))) {
            // This priority level is active, use its color
            m_current_color[0] = m_colors[i][0];
            m_current_color[1] = m_colors[i][1];
            m_current_color[2] = m_colors[i][2];
            m_current_color[3] = m_colors[i][3];
            updated = true;
            break;
        }
    }
    
    if (updated) {
        write_color_internal(m_current_color[0], m_current_color[1], m_current_color[2], m_current_color[3]);
    }
}