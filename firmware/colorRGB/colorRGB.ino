#include <Wire.h>
#include <Adafruit_TCS34725.h>

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS,TCS34725_GAIN_4X);
uint16_t r1, g1, b1, c1;
void setup() {
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

void loop() {
  
  uint16_t r, g, b, c;

  tcs.getRawData(&r, &g, &b, &c);

  Serial.print("R: "); Serial.print(r);
  Serial.print("  G: "); Serial.print(g);
  Serial.print("  B: "); Serial.print(b);
  Serial.print("  C: "); Serial.println(c);
  if (abs(r - r1) <= 10 && abs(g - g1) <= 10 && abs(b - b1) <= 10) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);
  }
}
