// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

// MODIFIED FROM:

/*
  xlgt_03_sm16716.ino - sm16716 three channel led support for Tasmota

  Copyright (C) 2020  Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************************\
 * SM16716 - Controlling RGB over a synchronous serial line
 * Copyright (C) 2020  Gabor Simon
 *
 * Source: https://community.home-assistant.io/t/cheap-uk-wifi-bulbs-with-tasmota-teardown-help-tywe3s/40508/27
\*********************************************************************************************/

#define GPIO_SM16716_CLK    5
#define GPIO_SM16716_DAT    4
#define GPIO_SM16716_SEL    13
#define GPIO_WHITE_PWM      12

struct SM16716 {
  uint8_t pin_clk = 0;
  uint8_t pin_dat = 0;
  uint8_t pin_sel = 0;
} Sm16716;

void SM16716_SendBit(uint8_t v)
{
  /* NOTE:
   * According to the spec sheet, max freq is 30 MHz, that is 16.6 ns per high/low half of the
   * clk square wave. That is less than the overhead of 'digitalWrite' at this clock rate,
   * so no additional delays are needed yet. */

  digitalWrite(Sm16716.pin_dat, (v != 0) ? HIGH : LOW);
  //delayMicroseconds(1);
  digitalWrite(Sm16716.pin_clk, HIGH);
  //delayMicroseconds(1);
  digitalWrite(Sm16716.pin_clk, LOW);
}

void SM16716_SendByte(uint8_t v)
{
  uint8_t mask;

  for (mask = 0x80; mask; mask >>= 1) {
    SM16716_SendBit(v & mask);
  }
}

void SM16716_Update(uint8_t duty_r, uint8_t duty_g, uint8_t duty_b)
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

void SM16716_Init(void)
{
  for (uint32_t t_init = 0; t_init < 50; ++t_init) {
    SM16716_SendBit(0);
  }
}

/********************************************************************************************/

void LEDSetup(void)
{
    Sm16716.pin_clk = GPIO_SM16716_CLK;
    Sm16716.pin_dat = GPIO_SM16716_DAT;
    Sm16716.pin_sel = GPIO_SM16716_SEL;

    // init sm16716
    pinMode(Sm16716.pin_clk, OUTPUT);
    digitalWrite(Sm16716.pin_clk, LOW);

    pinMode(Sm16716.pin_dat, OUTPUT);
    digitalWrite(Sm16716.pin_dat, LOW);

    pinMode(GPIO_WHITE_PWM, OUTPUT);
    digitalWrite(GPIO_WHITE_PWM, LOW);

    pinMode(Sm16716.pin_sel, OUTPUT);
    digitalWrite(Sm16716.pin_sel, HIGH);

    SM16716_Init();

    // do our best to prevent the leds from flashing when we enable the chip
    unsigned long start = millis();
    while (millis() < start + 2) {
      SM16716_Update(0,0,0);
    }
}

void setLEDColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  SM16716_Update(r, g, b);
  analogWrite(GPIO_WHITE_PWM, w);
}


