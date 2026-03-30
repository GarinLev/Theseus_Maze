#include <Wire.h>
#include <Adafruit_VL53L0X.h>

const uint8_t xshutPins[] = {32, 33, 34, 14, 15, 16};
const int numSensors = sizeof(xshutPins);

Adafruit_VL53L0X sensor;
uint8_t sensorAddresses[numSensors];

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();
  Wire.setClock(100000);
  for (int i = 0; i < numSensors; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }
  delay(100);
  for (int i = 0; i < numSensors; i++) {
    Serial.print("Датчик ");
    Serial.print(i);
    Serial.print(" (пин ");
    Serial.print(xshutPins[i]);
    Serial.println("):");
    digitalWrite(xshutPins[i], HIGH);
    delay(300);
    Wire.beginTransmission(0x29);
    if (Wire.endTransmission() != 0) {
      Serial.println("Не отвечает");
      sensorAddresses[i] = 0;
      digitalWrite(xshutPins[i], LOW);
      continue;
    }
    Serial.println("ответил");

    if (!sensor.begin(0x29)) {
      Serial.println("Ошибка инициализации");
      sensorAddresses[i] = 0;
      digitalWrite(xshutPins[i], LOW);
      continue;
    }
    Serial.println("Инициализация успешна");
    uint8_t newAddr = 0x30 + i;
    sensor.setAddress(newAddr);
    sensorAddresses[i] = newAddr;
    Serial.print("Новый адрес: 0x");
    Serial.println(newAddr, HEX);

    digitalWrite(xshutPins[i], LOW);
    delay(50);
  }

  Serial.println("Инициализация завершена.");
  Serial.println();
}
VL53L0X_RangingMeasurementData_t measure;

int cmp(const void* a, const void* b) {
  int ia = *(int*)a;
  int ib = *(int*)b;
  return (ia > ib) - (ia < ib); // или return ia - ib;
}

int filter1() {
  int dist[5];
  for (int i = 0; i < 5; i++) {
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      dist[i] = measure.RangeMilliMeter ;
    }
    else {
      dist[i] = 0;
    }
    delay(20);
  }
  int distance = 0;
  for (int i = 0; i < 5; i++) {
    distance += dist[i];
  }
  return distance / 5;
}

int cmpInt(const void* a, const void* b) {
  return (*(int*)a - * (int*)b);
}

int filter2() {
  int dist[5];
  for (int i = 0; i < 5; i++) {
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      dist[i] = measure.RangeMilliMeter ;
    }
    else {
      dist[i] = 0;
    }
    delay(20);
  }
  qsort(dist, 5, sizeof(int), cmp);
  return dist[2];
}



void loop() {
  for (int i = 0; i < numSensors; i++) {
    if (sensorAddresses[i] == 0) {
      Serial.print("0 ");
      continue;
    }

    digitalWrite(xshutPins[i], HIGH);
    delay(100);
    if (!sensor.begin(sensorAddresses[i])) {
      digitalWrite(xshutPins[i], LOW);
        continue;
    }


    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      Serial.print(measure.RangeMilliMeter);
    } else {
      Serial.print("0");
    }
    Serial.print(" ");

    digitalWrite(xshutPins[i], LOW);
    delay(10);
  }
  Serial.println();
  delay(200);
}
