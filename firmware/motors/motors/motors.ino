#define MOT1_IN1 4
#define MOT1_IN2 5
#define MOT2_IN1 6
#define MOT2_IN2 7
#define MOT3_IN1 8
#define MOT3_IN2 9
#define MOT4_IN1 10
#define MOT4_IN2 15

#define ENC1_A 2
#define ENC1_B 22
#define ENC2_A 3
#define ENC2_B 23
#define ENC3_A 18
#define ENC3_B 24
#define ENC4_A 19
#define ENC4_B 25

#define KP 1.5
#define KD 0.3
#define DT 0.01
volatile long encoder1 = 0;
volatile long encoder2 = 0;
volatile long encoder3 = 0;
volatile long encoder4 = 0;

void encoder1_isr() {
  if (digitalRead(ENC1_B) == HIGH) 
    encoder1--; 
  else 
    encoder1++;
}
void encoder2_isr() {
  if (digitalRead(ENC2_B) == HIGH) 
    encoder2++; 
  else 
      encoder2--;
}
void encoder3_isr() {
  if (digitalRead(ENC3_B) == HIGH) 
    encoder3++; 
  else 
    encoder3--;
}
void encoder4_isr() {
  if (digitalRead(ENC4_B) == HIGH) 
    encoder4--; 
  else 
    encoder4++;
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



void moveForward(long t, int speed) {
  resetEncoders();
  float errold1 = 0, errold2 = 0, errold3 = 0, errold4 = 0;
  while (1) {
    long a = (encoder1 + encoder2 + encoder3 + encoder4) / 4;
    if (a >= t) break;
    float err1 = a - encoder1, err2 = a - encoder2, err3 = a - encoder3, err4 = a - encoder4;
    float d1 = (err1 - errold1) / DT, d2 = (err2 - errold2) / DT, d3 = (err3 - errold3) / DT, d4 = (err4 - errold4) / DT;
    int s1 = speed + KP * err1 + KD * d1;
    int s2 = speed + KP * err2 + KD * d2;
    int s3 = speed + KP * err3 + KD * d3;
    int s4 = speed + KP * err4 + KD * d4;
    setMotors(constrain(s1, -255, 255), constrain(s2, -255, 255), constrain(s3, -255, 255), constrain(s4, -255, 255));
    errold1 = err1; errold2 = err2; errold3 = err3; errold4 = err4;
    delay(10);
  }
  stopMotors();
}

void turnRight(long d, int speed) {
  resetEncoders();
  float errold1 = 0, errold2 = 0, errold3 = 0, errold4 = 0;
  while (1) {
    long df = (encoder1 + encoder3) - (encoder2 + encoder4);
    if (df >= d) break;
    float al = (encoder1 + encoder3) / 2.0, ar = (encoder2 + encoder4) / 2.0;
    float err1 = al - encoder1, err3 = al - encoder3;
    float err2 = ar - encoder2, err4 = ar - encoder4;
    float d1 = (err1 - errold1) / DT, d3 = (err3 - errold3) / DT;
    float d2 = (err2 - errold2) / DT, d4 = (err4 - errold4) / DT;
    int s1 =  speed + KP * err1 + KD * d1;
    int s3 =  speed + KP * err3 + KD * d3;
    int s2 = -speed + KP * err2 + KD * d2;
    int s4 = -speed + KP * err4 + KD * d4;
    setMotors(constrain(s1, -255, 255), constrain(s2, -255, 255), constrain(s3, -255, 255), constrain(s4, -255, 255));
    errold1 = err1; errold2 = err2; errold3 = err3; errold4 = err4;
    delay(10);
  }
  stopMotors();
}

void turnLeft(long d, int speed) {
  resetEncoders();
  float errold1 = 0, errold2 = 0, errold3 = 0, errold4 = 0;
  while (1) {
    long df = (encoder1 + encoder3) - (encoder2 + encoder4);
    if (df <= -d) break;
    float al = (encoder1 + encoder3) / 2.0, ar = (encoder2 + encoder4) / 2.0;
    float err1 = al - encoder1, err3 = al - encoder3;
    float err2 = ar - encoder2, err4 = ar - encoder4;
    float d1 = (err1 - errold1) / DT, d3 = (err3 - errold3) / DT;
    float d2 = (err2 - errold2) / DT, d4 = (err4 - errold4) / DT;
    int s1 = -speed + KP * err1 + KD * d1;
    int s3 = -speed + KP * err3 + KD * d3;
    int s2 =  speed + KP * err2 + KD * d2;
    int s4 =  speed + KP * err4 + KD * d4;
    setMotors(constrain(s1, -255, 255), constrain(s2, -255, 255), constrain(s3, -255, 255), constrain(s4, -255, 255));
    errold1 = err1; errold2 = err2; errold3 = err3; errold4 = err4;
    delay(10);
  }
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
  attachInterrupt(0, encoder1_isr, RISING);
  attachInterrupt(1, encoder2_isr, RISING);
  attachInterrupt(4, encoder3_isr, RISING);
  attachInterrupt(5, encoder4_isr, RISING);
  
}

void loop() {
  moveForward(500, 150);
  stopMotors;
  delay(2000);
  turnRight(500, 200);
  stopMotors;
  delay(2000);
  turnLeft(500, 200);
  stopMotors;
  delay(2000);
}
