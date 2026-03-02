#include <Wire.h>
#include <Adafruit_VL53L0X.h>

const uint8_t xshutPins[] = {40, 41, 42, 43, 44, 45};
const int numSensors = sizeof(xshutPins);
Adafruit_VL53L0X sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  for (int i = 0; i < numSensors; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }
  delay(10);
}

void loop() {
  for (int i = 0; i < numSensors; i++) {
    digitalWrite(xshutPins[i], HIGH);
    delay(10);
    if (!sensor.begin(0x29)) {
      digitalWrite(xshutPins[i], LOW);
    }
    sensor.setMeasurementTimingBudget(20000);
    VL53L0X_RangingMeasurementData_t measure;
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      Serial.print(measure.RangeMilliMeter);
    } else {
      Serial.print("0");
    }
    digitalWrite(xshutPins[i], LOW);
    delay(5);
  }
  Serial.println();
  delay(200);
}
