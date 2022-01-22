/**
 * Copyright 2022 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

#include <LedControl.h>

#define LED_DIN 13  // nodemcu v3 pin D7
#define LED_CS 2  // nodemcu v3 pin D4
#define LED_CLK 14  // nodemcu v3 pin D5

LedControl lc = LedControl(LED_DIN, LED_CLK, LED_CS, 4);

const int INTERVAL_MS = 10;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  for (int i = 0; i < lc.getDeviceCount(); i++) {
        lc.shutdown(i, false);  // It's in power-saving mode on startup
        lc.setIntensity(i, 1);  // Set the brightness to medium value (max 15)
        lc.clearDisplay(0);  // and clear the display
    }
}

void loop() {
      delay(INTERVAL_MS);
      updatePixel();
}

void updatePixel() {
  int screen = random(4);
  int row = random(8);
  int column = random(8);
  int state = random(2);

  lc.setLed(screen, row, column, state);
}
