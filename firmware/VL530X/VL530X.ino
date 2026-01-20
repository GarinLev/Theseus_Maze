#include <Wire.h>
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Start");

  Wire.begin(); // для Mega важно явно

  if (!lox.begin()) {
    Serial.println("VL53L0X NOT FOUND");
    while (1);
  }

  Serial.println("VL53L0X OK");
}

void loop() {
}

/*
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin();
  Serial.println("I2C scan...");
}

void loop() {
  byte error, address;
  int found = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found at 0x");
      Serial.println(address, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C devices found");
  }

  delay(3000);
}
*/
