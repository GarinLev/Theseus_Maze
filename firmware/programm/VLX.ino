void setupVLX() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();
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

    // Проверяем наличие устройства по адресу 0x29 (на нём может висеть RGB, но это не страшно)
    Wire.beginTransmission(0x29);
    if (Wire.endTransmission() != 0) {
      Serial.println("Не отвечает");
      sensorAddresses[i] = 0;
      digitalWrite(xshutPins[i], LOW);
      continue;
    }
    Serial.println("ответил");

    // Вручную меняем адрес, не используя sensor.begin (чтобы не конфликтовать с RGB)
    uint8_t newAddr = 0x30 + i;
    bool success = false;
    // Пытаемся изменить адрес, отправляя команду напрямую через Wire
    Wire.beginTransmission(0x29);
    Wire.write(0x8A);          // регистр I2C_SLAVE_DEVICE_ADDRESS
    Wire.write(newAddr);
    if (Wire.endTransmission() == 0) {
      delay(10);
      // Проверяем, что устройство откликается на новом адресе
      Wire.beginTransmission(newAddr);
      if (Wire.endTransmission() == 0) {
        success = true;
      }
    }
    if (success) {
      Serial.println("Адрес изменён");
      sensorAddresses[i] = newAddr;
    } else {
      Serial.println("Ошибка изменения адреса");
      sensorAddresses[i] = 0;
    }

    digitalWrite(xshutPins[i], LOW);
    delay(50);
  }

  Serial.println("Инициализация лазеров завершена.");
  Serial.println();
}


int cmp(const void* a, const void* b) {
  int ia = *(int*)a;
  int ib = *(int*)b;
  return (ia > ib) - (ia < ib);
}

int filter1() {
  int dist[5];
  for (int i = 0; i < 5; i++) {
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      dist[i] = measure.RangeMilliMeter;
    } else {
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
  return (*(int*)a - *(int*)b);
}

int filter2() {
  int dist[5];
  for (int i = 0; i < 5; i++) {
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      dist[i] = measure.RangeMilliMeter;
    } else {
      dist[i] = 0;
    }
    delay(20);
  }
  qsort(dist, 5, sizeof(int), cmp);
  return dist[2];
}

int get_distance(int i) {
  if (sensorAddresses[i] == 0) {
    return 0;
  }

  digitalWrite(xshutPins[i], HIGH);
  delay(100);

  if (!sensor.begin(sensorAddresses[i])) {
    digitalWrite(xshutPins[i], LOW);
    return 0;
  }
  int distance;
  sensor.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distance = measure.RangeMilliMeter;
  } else {
    distance = 0;
  }
  digitalWrite(xshutPins[i], LOW);
  delay(10);
  return distance;
}

bool checkRight() {
  Serial.println(get_distance(2));
  if (get_distance(2) < 200 && get_distance(2) != 0) {
    return true;
  } else {
    return false;
  }
}

bool checkForward() {
  if (get_distance(0) < 200 && get_distance(0) != 0) {
    return true;
  } else {
    return false;
  }
}
