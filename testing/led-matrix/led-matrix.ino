/**
 * Copyright 2022 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

/** 
 *  Following https://www.instructables.com/Interface-LED-Dot-Matrix-8x8-With-NodeMCU/ but with different pins
 *  and http://wayoda.github.io/LedControl/pages/software
  */


#include <LedControl.h>

#define LED_DIN 13  // nodemcu v3 pin D7
#define LED_CS 2  // nodemcu v3 pin D4
#define LED_CLK 14  // nodemcu v3 pin D5

LedControl lc = LedControl(LED_DIN, LED_CLK, LED_CS, 4);


void setup() {
  Serial.begin(115200);

  for (int i = 0; i < lc.getDeviceCount(); i++) {
        lc.shutdown(i, false);  // It's in power-saving mode on startup
        lc.setIntensity(i, 6);  // Set the brightness to medium value (max 15)
        lc.clearDisplay(0);  // and clear the display
    }
}

void loop() {
  byte a[8] = {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF};  // L
    byte b[8] = {0xFF, 0xFF, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF};  // I
    byte c[8] = {0x7F, 0xFF, 0xC0, 0xDF, 0xDF, 0xC3, 0x7F, 0x3F};  // G
    byte d[8] = {0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3};  // H
    byte e[8] = {0xFF, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};  // T

    byte f[8] = {0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3};  // H
    byte g[8] = {0x3C, 0x7E, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E, 0x3C};  // O
    byte h[8] = {0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF};  // U
    byte i[8] = {0x7F, 0xFE, 0xC0, 0xFE, 0x7F, 0x03, 0x7F, 0xFE};  // S
    byte j[8] = {0xFF, 0xFF, 0xC0, 0xF8, 0xF8, 0xC0, 0xFF, 0xFF};  // E
    printByte(a);
    delay(1000);
    printByte(b);
    delay(1000);
    printByte(c);
    delay(1000);
    printByte(d);
    delay(1000);
    printByte(e);
    delay(1000);
    printByte(f);
    delay(1000);
    printByte(g);
    delay(1000);
    printByte(h);
    delay(1000);
    printByte(i);
    delay(1000);
    printByte(j);
    delay(1000);
}

void printByte(byte character[]) {
  int i = 0;
  for (i=0; i < 8; i++) {
    lc.setRow(0, i, character[i]);
    lc.setRow(1, i, character[i]);
    lc.setRow(2, i, character[i]);
    lc.setRow(3, i, character[i]);
  }
}
