#include <Wire.h>

void setup() {
  Wire.begin();
  Wire.setClock(100000); // принудительно 100 кГц
  Serial.begin(115200);
  Serial.println("\nAdvanced I2C Scanner");
}

void loop() {
  byte error;
  int count = 0;
  
  Serial.print("Scanning (100kHz)... ");
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("\nFound at 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      count++;
      
      // Дополнительная проверка: попробовать прочитать 1 байт
      Wire.requestFrom(addr, 1);
      if (Wire.available()) {
        Serial.print(" (responds to read)");
      }
    }
  }
  
  if (count == 0) Serial.println("No devices found");
  else Serial.println("\nScan complete");
  
  delay(3000);
}
