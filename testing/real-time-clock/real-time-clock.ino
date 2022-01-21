/**
 * Copyright 2022 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

/**
 * Following guide at https://cyberblogspot.com/how-to-connect-a-ds3231-to-nodemcu-v3/
 */

#include <DS3232RTC.h>

DS3232RTC myRTC;

void setup() {
  Serial.begin(115200);
  myRTC.begin();
}

void loop() {
  display();
  delay(10000);
}

void display()
{
    char buf[40];
    time_t t = myRTC.get();
    float celsius = myRTC.temperature() / 4.0;
    sprintf(buf, "%.2d:%.2d:%.2d %.2d%s%d ",
        hour(t), minute(t), second(t), day(t), monthShortStr(month(t)), year(t));
    Serial.print(buf);
    Serial.print(celsius);
    Serial.print("C ");
    Serial.println();
}
