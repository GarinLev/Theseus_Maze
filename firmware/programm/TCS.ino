int r1, g1, b1, c1;
int calibrationStep = -1;
void setupTCS() {
  // Serial уже инициализирован в setupVLX, поэтому здесь не вызываем
  pinMode(13, OUTPUT);
  if (tcs.begin()) {
    Serial.println("найден");
  } else {
    Serial.println("не найден");
    while (1);
  }

  tcs.getRawData(&r1, &g1, &b1, &c1);

  Serial.print("R: "); Serial.print(r1);
  Serial.print("  G: "); Serial.print(g1);
  Serial.print("  B: "); Serial.print(b1);
  Serial.print("  C: "); Serial.println(c1);

  delay(500);
}

void waitButtonPress() {
  pinMode(CAL_BUTTON, INPUT_PULLUP);
  while (digitalRead(CAL_BUTTON) == HIGH) delay(50);
  delay(100);
  while (digitalRead(CAL_BUTTON) == LOW) delay(10);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
}

void calibrateColors() {
  tcs.getRawData(&black_r, &black_g, &black_b, NULL);
  Serial.print("Чёрный: R="); Serial.print(black_r);
  Serial.print(" G="); Serial.print(black_g);
  Serial.print(" B="); Serial.println(black_b);

  waitButtonPress();
  tcs.getRawData(&red_r, &red_g, &red_b, NULL);

  waitButtonPress();
  tcs.getRawData(&blue_r, &blue_g, &blue_b, NULL);

}

bool isBlack() {
  get_color(); // обновляет r1, g1, b1
  long dr = (long)r1 - black_r;
  long dg = (long)g1 - black_g;
  long db = (long)b1 - black_b;
  long distSq = dr*dr + dg*dg + db*db;
  return distSq < COLOR_THRESHOLD;
}

bool isBlue() {
  get_color();
  long dr = (long)r1 - blue_r;
  long dg = (long)g1 - blue_g;
  long db = (long)b1 - blue_b;
  long distSq = dr*dr + dg*dg + db*db;
  return distSq < COLOR_THRESHOLD;
}

bool isRed() {
  get_color();
  long dr = (long)r1 - red_r;
  long dg = (long)g1 - red_g;
  long db = (long)b1 - red_b;
  long distSq = dr*dr + dg*dg + db*db;
  return distSq < COLOR_THRESHOLD;
}
