int r1, g1, b1, c1;
void setupTCS() {
  Serial.begin(9600);
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
void get_color(){
  int r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  return r, g, b, c;
}
bool check_floor();
