#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X sensor = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);

  if (!sensor.begin()) {
    Serial.println("Ошибка инициализации датчика!");
    while (1);
  }
  Serial.println("Датчик готов!");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  sensor.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    Serial.print("Расстояние: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.println(" mm");
  } else {
    Serial.println("Вне диапазона");
  }

  delay(500);
}
