/**
 * Copyright 2022 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

/** 
 *  Following https://www.instructables.com/Interface-LED-Dot-Matrix-8x8-With-NodeMCU/ but with different pins
 *  and http://wayoda.github.io/LedControl/pages/software
 *  
 *  and pixel design guide from https://randomnerdtutorials.com/guide-for-8x8-dot-matrix-max7219-with-arduino-pong-game/
  */


#include <LedControl.h>

#define LED_DIN 13  // nodemcu v3 pin D7
#define LED_CS 2  // nodemcu v3 pin D4
#define LED_CLK 14  // nodemcu v3 pin D5

LedControl lc = LedControl(LED_DIN, LED_CLK, LED_CS, 4);

const byte ON_AIR_BLOCK_0[] = {
  B01110011,
  B10001011,
  B10001010,
  B10001010,
  B10001010,
  B10001010,
  B10001010,
  B01110010
};

const byte ON_AIR_BLOCK_1[] = {
  B00010000,
  B00010000,
  B10010000,
  B10010000,
  B01010000,
  B01010000,
  B00110000,
  B00110000
};

const byte ON_AIR_BLOCK_2[] = {
  B00110001,
  B00110000,
  B01001000,
  B01001000,
  B01001000,
  B11111100,
  B10000100,
  B10000101
};

const byte ON_AIR_BLOCK_3[] = {
  B11011110,
  B10010001,
  B10010001,
  B10010001,
  B10011110,
  B10010010,
  B10010001,
  B11010001
};

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < lc.getDeviceCount(); i++) {
        lc.shutdown(i, false);  // It's in power-saving mode on startup
        lc.setIntensity(i, 1);  // Set the brightness to medium value (max 15)
        lc.clearDisplay(0);  // and clear the display
    }
}

void loop() {
      delay(1000);

      print8x8(3, ON_AIR_BLOCK_0);
      print8x8(2, ON_AIR_BLOCK_1);
      print8x8(1, ON_AIR_BLOCK_2);
      print8x8(0, ON_AIR_BLOCK_3);
}

void print8x8(int screenId, const byte pixels[]) {
  for (int i = 0; i < 8; i++) {
    lc.setRow(screenId, i, pixels[i]);
  }
}
