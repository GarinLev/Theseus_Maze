// Пины драйверов моторов
#define MOT1_IN1 4
#define MOT1_IN2 5
#define MOT2_IN1 6
#define MOT2_IN2 7
#define MOT3_IN1 8
#define MOT3_IN2 9
#define MOT4_IN1 10
#define MOT4_IN2 11

// Пины энкодеров
#define ENC1_A 2
#define ENC1_B 22
#define ENC2_A 3
#define ENC2_B 23
#define ENC3_A 18
#define ENC3_B 24
#define ENC4_A 19
#define ENC4_B 25
volatile long encoder1 = 0;
volatile long encoder2 = 0;
volatile long encoder3 = 0;
volatile long encoder4 = 0;

void encoder1_isr() {
  if (digitalRead(ENC1_B) == HIGH) encoder1++; else encoder1--;
}
void encoder2_isr() {
  if (digitalRead(ENC2_B) == HIGH) encoder2++; else encoder2--;
}
void encoder3_isr() {
  if (digitalRead(ENC3_B) == HIGH) encoder3++; else encoder3--;
}
void encoder4_isr() {
  if (digitalRead(ENC4_B) == HIGH) encoder4++; else encoder4--;
}


void setMotor1(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT1_IN1, 0);
    analogWrite(MOT1_IN2, pwm);
  } else {
    analogWrite(MOT1_IN1, -pwm);
    analogWrite(MOT1_IN2, 0);
  }
}

void setMotor2(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT2_IN1, pwm);
    analogWrite(MOT2_IN2, 0);
  } else {
    analogWrite(MOT2_IN1, 0);
    analogWrite(MOT2_IN2, -pwm);
  }
}

void setMotor3(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT3_IN1, 0);
    analogWrite(MOT3_IN2, pwm);
  } else {
    analogWrite(MOT3_IN1, -pwm);
    analogWrite(MOT3_IN2, 0);
  }
}

void setMotor4(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT4_IN1, pwm);
    analogWrite(MOT4_IN2, 0);
  } else {
    analogWrite(MOT4_IN1, 0);
    analogWrite(MOT4_IN2, -pwm);
  }
}

// Установка скоростей всех моторов сразу
void setMotors(int s1, int s2, int s3, int s4) {
  setMotor1(s1);
  setMotor2(s2);
  setMotor3(s3);
  setMotor4(s4);
}

// Остановка всех моторов
void stopMotors() {
  setMotors(0, 0, 0, 0);
}
void resetEncoders() {
  encoder1 = 0;
  encoder2 = 0;
  encoder3 = 0;
  encoder4 = 0;
}


void moveForwardToTarget(long target, int speed) {
  resetEncoders();
  speed = constrain(speed, 0, 255);
  while ((encoder1 + encoder2 + encoder3 + encoder4) / 4.0 < target) {
    setMotors(speed, speed, speed, speed);
    delay(10);
  }
  stopMotors();
}

void turnRightToTarget(long targetDiff, int speed) {
  resetEncoders();
  speed = constrain(speed, 0, 255);
  long leftSum, rightSum, diff;
  do {
    leftSum = encoder1 + encoder3;
    rightSum = encoder2 + encoder4;
    diff = leftSum - rightSum;ъ
    setMotors(speed, -speed, speed, -speed);
    delay(10);
  } while (diff < targetDiff);
  stopMotors();
}

void turnLeftToTarget(long targetDiff, int speed) {
  resetEncoders();
  speed = constrain(speed, 0, 255);
  long leftSum, rightSum, diff;
  do {
    leftSum = encoder1 + encoder3;
    rightSum = encoder2 + encoder4;
    diff = leftSum - rightSum;
    setMotors(-speed, speed, -speed, speed);
    delay(10);
  } while (diff > -targetDiff);
  stopMotors();
}


void setup() {
  Serial.begin(115200);

  // Настройка пинов моторов
  pinMode(MOT1_IN1, OUTPUT);
  pinMode(MOT1_IN2, OUTPUT);
  pinMode(MOT2_IN1, OUTPUT);
  pinMode(MOT2_IN2, OUTPUT);
  pinMode(MOT3_IN1, OUTPUT);
  pinMode(MOT3_IN2, OUTPUT);
  pinMode(MOT4_IN1, OUTPUT);
  pinMode(MOT4_IN2, OUTPUT);
  pinMode(ENC1_A, INPUT_PULLUP);
  pinMode(ENC1_B, INPUT_PULLUP);
  pinMode(ENC2_A, INPUT_PULLUP);
  pinMode(ENC2_B, INPUT_PULLUP);
  pinMode(ENC3_A, INPUT_PULLUP);
  pinMode(ENC3_B, INPUT_PULLUP);
  pinMode(ENC4_A, INPUT_PULLUP);
  pinMode(ENC4_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC1_A), encoder1_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), encoder2_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC3_A), encoder3_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC4_A), encoder4_isr, RISING);
  
}

void loop() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("Encoders: ");
    Serial.print(encoder1); Serial.print(" ");
    Serial.print(encoder2); Serial.print(" ");
    Serial.print(encoder3); Serial.print(" ");
    Serial.println(encoder4);
  }
}
