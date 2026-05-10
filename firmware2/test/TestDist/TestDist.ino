#include <Wire.h>
#include <VL53L0X_mod.h>

const int numSensors = 6;
VL53L0X_mod sensors[numSensors];

// Пины XSHUT для Arduino Mega
const int xshutPins[numSensors] = {32, 33, 34, 35, 36, 37}; 
// Массив уникальных адресов
const uint8_t addresses[numSensors] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

void setup() {
  Serial.begin(115200);
  Wire.begin(); // На Mega это SDA (20) и SCL (21)

  // 1. Сначала выключаем все датчики, прижимая XSHUT к земле
  for (int i = 0; i < numSensors; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }
  delay(10);

  // 2. По очереди включаем и настраиваем каждый датчик
  for (int i = 0; i < numSensors; i++) {
    digitalWrite(xshutPins[i], HIGH); // "Будим" датчик i
    delay(10);
    
    if (!sensors[i].init()) {
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.println(" NOT FOUND!");
    } else {
      sensors[i].setAddress(addresses[i]); // Меняем адрес с 0x29 на новый
      sensors[i].setTimeout(500);
      sensors[i].startContinuous(100); // Опрос каждые 100мс
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.println(" ready.");
    }
  }
}

void loop() {
  for (int i = 0; i < numSensors; i++) {
    uint16_t dist;
    // Используем ваш неблокирующий метод
    if (sensors[i].readRangeNoBlocking(dist)) {
      Serial.print("S");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(dist);
      Serial.print(" ");
    }
  }
  Serial.println();
}
