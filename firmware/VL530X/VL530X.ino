#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X sensor = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  while (!Serial); // Ждём подключения монитора порта (для Leonardo/Micro, но оставлю)

  Wire.begin();
  delay(100); // Даём датчику время на запуск

  Serial.println("Инициализация VL53L0X...");
  if (!sensor.begin()) {
    Serial.println("Ошибка! Датчик не найден.");
    Serial.println("Проверьте: питание 3.3V, провода SDA/SCL, подтягивающие резисторы (обычно уже есть).");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Датчик готов!");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  sensor.rangingTest(&measure, false); // false — без отладочного вывода

  if (measure.RangeStatus != 4) { // 4 = вне диапазона
    Serial.print("Расстояние: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.println(" mm");
  } else {
    Serial.println("Вне диапазона");
  }

  delay(500);
}
