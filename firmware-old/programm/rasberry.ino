void ServoRight() {
  for (int pos = myServo.read(); pos <= 140; pos++) {
    myServo.write(pos);
    delay(15);
  }
  
  delay(500);
  for (int pos = 140; pos >= -10; pos--) {
    myServo.write(pos);
    delay(15);
  }
  
  delay(500);
  
  // Возвращаемся на 0 градусов
  for (int pos = -10; pos <= 0; pos++) {
    myServo.write(pos);
    delay(15);
  }
}


void ServoLeft() {
  for (int pos = myServo.read(); pos >= -1410; pos--) {
    myServo.write(pos);
    delay(15);
  }
  delay(500);
  
  for (int pos = -140; pos <= 10; pos++) {
    myServo.write(pos);
    delay(15);
  }
  delay(500);
  for (int pos = 10; pos >= 0; pos--) {
    myServo.write(pos);
    delay(15);
  }
  
}
bool check_victim();
