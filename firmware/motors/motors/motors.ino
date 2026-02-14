// Пины энкодеров
#define ENC1_A 2
#define ENC1_B 22
#define ENC2_A 3
#define ENC2_B 23
#define ENC3_A 18
#define ENC3_B 24
#define ENC4_A 19
#define ENC4_B 25

// Пины драйверов
#define MOT1_IN1 4
#define MOT1_IN2 5
#define MOT2_IN1 6
#define MOT2_IN2 7
#define MOT3_IN1 8
#define MOT3_IN2 9
#define MOT4_IN1 10
#define MOT4_IN2 11

// Коэффициенты PID
#define KP 2.0
#define KI 0.05
#define KD 0.1
#define MAX_PWM 255
#define MAX_CORRECTION 50

// Счётчики энкодеров
volatile long encoder1 = 0;
volatile long encoder2 = 0;
volatile long encoder3 = 0;
volatile long encoder4 = 0;

// Переменные для PID
float lastError1 = 0, lastError2 = 0, lastError3 = 0, lastError4 = 0;
float integral1 = 0, integral2 = 0, integral3 = 0, integral4 = 0;
unsigned long lastTime = 0;

// ========== ПРЕРЫВАНИЯ ЭНКОДЕРОВ ==========
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

// ========== УПРАВЛЕНИЕ МОТОРАМИ ==========
// Левые моторы (1 и 3) инвертированы, чтобы при положительном PWM ехать вперёд
void setMotor1(int pwm) {
  pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
  if (pwm >= 0) {
    analogWrite(MOT1_IN1, 0);
    analogWrite(MOT1_IN2, pwm);
  } else {
    analogWrite(MOT1_IN1, -pwm);
    analogWrite(MOT1_IN2, 0);
  }
}

void setMotor2(int pwm) {
  pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
  if (pwm >= 0) {
    analogWrite(MOT2_IN1, pwm);
    analogWrite(MOT2_IN2, 0);
  } else {
    analogWrite(MOT2_IN1, 0);
    analogWrite(MOT2_IN2, -pwm);
  }
}

void setMotor3(int pwm) {
  pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
  if (pwm >= 0) {
    analogWrite(MOT3_IN1, 0);
    analogWrite(MOT3_IN2, pwm);
  } else {
    analogWrite(MOT3_IN1, -pwm);
    analogWrite(MOT3_IN2, 0);
  }
}

void setMotor4(int pwm) {
  pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
  if (pwm >= 0) {
    analogWrite(MOT4_IN1, pwm);
    analogWrite(MOT4_IN2, 0);
  } else {
    analogWrite(MOT4_IN1, 0);
    analogWrite(MOT4_IN2, -pwm);
  }
}

// ========== ИНИЦИАЛИЗАЦИЯ ==========
void setupPins() {
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
}

void resetEncoders() {
  encoder1 = 0; encoder2 = 0; encoder3 = 0; encoder4 = 0;
  lastError1 = lastError2 = lastError3 = lastError4 = 0;
  integral1 = integral2 = integral3 = integral4 = 0;
}

void stopMotors() {
  setMotor1(0); setMotor2(0); setMotor3(0); setMotor4(0);
}

// ========== ФУНКЦИИ ДВИЖЕНИЯ ==========
// Движение вперёд с синхронизацией всех колёс
void moveForwardSync(int baseSpeed) {
  float avg = (encoder1 + encoder2 + encoder3 + encoder4) / 4.0;

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  if (dt <= 0) dt = 0.01;
  lastTime = now;

  float error1 = avg - encoder1;
  integral1 += error1 * dt;
  float deriv1 = (error1 - lastError1) / dt;
  float corr1 = KP * error1 + KI * integral1 + KD * deriv1;
  lastError1 = error1;
  corr1 = constrain(corr1, -MAX_CORRECTION, MAX_CORRECTION);
  setMotor1(baseSpeed + corr1);

  float error2 = avg - encoder2;
  integral2 += error2 * dt;
  float deriv2 = (error2 - lastError2) / dt;
  float corr2 = KP * error2 + KI * integral2 + KD * deriv2;
  lastError2 = error2;
  corr2 = constrain(corr2, -MAX_CORRECTION, MAX_CORRECTION);
  setMotor2(baseSpeed + corr2);

  float error3 = avg - encoder3;
  integral3 += error3 * dt;
  float deriv3 = (error3 - lastError3) / dt;
  float corr3 = KP * error3 + KI * integral3 + KD * deriv3;
  lastError3 = error3;
  corr3 = constrain(corr3, -MAX_CORRECTION, MAX_CORRECTION);
  setMotor3(baseSpeed + corr3);

  float error4 = avg - encoder4;
  integral4 += error4 * dt;
  float deriv4 = (error4 - lastError4) / dt;
  float corr4 = KP * error4 + KI * integral4 + KD * deriv4;
  lastError4 = error4;
  corr4 = constrain(corr4, -MAX_CORRECTION, MAX_CORRECTION);
  setMotor4(baseSpeed + corr4);
}

// Поворот с синхронизацией (turnFactor >0 — вправо, <0 — влево)
void turnSync(int baseSpeed, float turnFactor) {
  float leftAvg = (encoder1 + encoder3) / 2.0;
  float rightAvg = (encoder2 + encoder4) / 2.0;
  float diff = leftAvg - rightAvg;
  float targetDiff = turnFactor * 50;
  float error = diff - targetDiff;

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  if (dt <= 0) dt = 0.01;
  lastTime = now;

  static float integralTurn = 0;
  static float lastErrorTurn = 0;
  integralTurn += error * dt;
  float derivTurn = (error - lastErrorTurn) / dt;
  lastErrorTurn = error;
  float correction = KP * error + KI * integralTurn + KD * derivTurn;
  correction = constrain(correction, -MAX_CORRECTION, MAX_CORRECTION);

  setMotor1(baseSpeed - correction);
  setMotor3(baseSpeed - correction);
  setMotor2(baseSpeed + correction);
  setMotor4(baseSpeed + correction);
}

// Поворот на месте
void rotateInPlace(int speed, bool clockwise) {
  if (clockwise) {
    setMotor1(speed);  setMotor3(speed);
    setMotor2(-speed); setMotor4(-speed);
  } else {
    setMotor1(-speed); setMotor3(-speed);
    setMotor2(speed);  setMotor4(speed);
  }
}

// ========== SETUP / LOOP ==========
void setup() {
  Serial.begin(115200);
  setupPins();

  attachInterrupt(digitalPinToInterrupt(ENC1_A), encoder1_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), encoder2_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC3_A), encoder3_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC4_A), encoder4_isr, RISING);

  lastTime = millis();
  resetEncoders();
}

void loop() {
  // Пример: едем вперёд со скоростью 150
  moveForwardSync(150);
  delay(10);

  // Вывод значений энкодеров для отладки
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
