void setupVLX() {
  // Сброс всех датчиков (LOW на XSHUT)
  for (int i = 0; i < sizeof(lox_shts) / sizeof(lox_shts[0]); i++) {
    pinMode(lox_shts[i], OUTPUT);
    digitalWrite(lox_shts[i], LOW);
    delay(10);
  }

  Serial.println("Настройка VL53L0X...");

  // По очереди включаем, меняем адрес, инициализируем
  for (int i = 0; i < sizeof(lox_shts) / sizeof(lox_shts[0]); i++) {
    Serial.print("Датчик ");
    Serial.print(i);
    delay(10);
    digitalWrite(lox_shts[i], HIGH);   // включаем питание датчика
    delay(10);
    lox[i].setAddress(lox_adresses[i]); // меняем I2C адрес
    if (!lox[i].init()) {
      Serial.println(" - Ошибка инициализации");
    } else {
      lox[i].startContinuous();         // непрерывный режим
      Serial.println(" - OK");
    }
    delay(10);
  }
  Serial.println("Все VL53L0X готовы");
}

/*int cmp(const void* a, const void* b) {
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
}*/

int get_distance(int i) {
  if (i < 0 || i >= 6) return 0;
  uint16_t dist = lox[i].readRangeContinuousMillimeters();
  // Проверка таймаута (если измерение невалидно)
  if (lox[i].timeoutOccurred()) return 0;
  return dist;
}


bool checkRight() {
  Serial.println(get_distance(2));
  if (get_distance(2) < 200 && get_distance(2) != 0) {
    return true;
  } else {
    return false;
  }
}
bool checkLeft() {
  Serial.println(get_distance(5));
  if (get_distance(5) < 200 && get_distance(5) != 0) {
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
