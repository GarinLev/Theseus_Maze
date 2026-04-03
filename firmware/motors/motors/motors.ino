#define MOT1_IN1 6
#define MOT1_IN2 7
#define MOT2_IN1 8
#define MOT2_IN2 9
#define MOT3_IN1 10
#define MOT3_IN2 11
#define MOT4_IN1 12
#define MOT4_IN2 4

#define KP 1.5
#define KD 0.3
#define DT 0.01

volatile int32_t enc1;
volatile int32_t enc2;
volatile int32_t enc3;
volatile int32_t enc4;

void ENCA() {
  enc1 = enc1 + (digitalRead(22) * 2 - 1);
}
void ENCB() {
  enc2 = enc2 + ((1 - digitalRead(23)) * 2 - 1);
}
void ENCC() {
  enc3 = enc3 + ((digitalRead(24)) * 2 - 1);
}
void ENCD() {
  enc4 = enc4 + ((1-digitalRead(25)) * 2 - 1);
}

void setMotor1(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT1_IN1, pwm);
    analogWrite(MOT1_IN2, 0);
  } else {
    analogWrite(MOT1_IN1, 0);
    analogWrite(MOT1_IN2, -pwm);
  }
}

void setMotor2(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT2_IN1, 0);
    analogWrite(MOT2_IN2, pwm);
  } else {
    analogWrite(MOT2_IN1, -pwm);
    analogWrite(MOT2_IN2, 0);
  }
}

void setMotor3(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT3_IN1, pwm);
    analogWrite(MOT3_IN2, 0);
  } else {
    analogWrite(MOT3_IN1, 0);
    analogWrite(MOT3_IN2, -pwm);
  }
}
void setMotor4(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (pwm >= 0) {
    analogWrite(MOT4_IN1, 0);
    analogWrite(MOT4_IN2, pwm);
  } else {
    analogWrite(MOT4_IN1, -pwm);
    analogWrite(MOT4_IN2, 0);
  }
}

void setMotors(int s1, int s2, int s3, int s4) {
  setMotor1(s1);
  setMotor2(s2);
  setMotor3(s3);
  setMotor4(s4);
}

void stopMotors() {
  setMotors(0, 0, 0, 0);
}

void resetEncoders() {
  enc1 = 0;
  enc2 = 0;
  enc3 = 0;
  enc4 = 0;
}

void moveForward(long t, int speed) {
  resetEncoders();
  float errold1 = 0, errold2 = 0, errold3 = 0, errold4 = 0;
  while (1) {
    Serial.print(enc1);
    Serial.print(" ");
    Serial.print(enc2);
    Serial.print(" ");
    Serial.print(enc3);
    Serial.print(" ");
    Serial.println(enc4);
    long a = (enc1 + enc2 + enc3 + enc4) / 4;
    if (a >= t) break;
    float err1 = a - enc1, err2 = a - enc2, err3 = a - enc3, err4 = a - enc4;
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
    long df = (enc1 + enc3) - (enc2 + enc4);
    if (df >= d) break;
    float al = (enc1 + enc4) / 2.0;
    float ar = (enc2 + enc3) / 2.0;
    float err1 = al - enc1, err3 = al - enc3;
    float err2 = ar - enc2, err4 = ar - enc4;
    float d1 = (err1 - errold1) / DT, d3 = (err3 - errold3) / DT;
    float d2 = (err2 - errold2) / DT, d4 = (err4 - errold4) / DT;
    int s1 =  speed + KP * err1 + KD * d1;
    int s3 = speed + KP * err3 + KD * d3;
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
    long df = (enc1 + enc3) - (enc2 + enc4);
    if (df <= -d) break;
    
    float al = (enc1 + enc3) / 2.0;
    float ar = (enc2 + enc4) / 2.0;
    
    float err1 = al - enc1, err3 = al - enc3;
    float err2 = ar - enc2, err4 = ar - enc4;
    
    float d1 = (err1 - errold1) / DT, d3 = (err3 - errold3) / DT;
    float d2 = (err2 - errold2) / DT, d4 = (err4 - errold4) / DT;
    
    int s1 = -speed + KP * err1 + KD * d1;
    int s3 = -speed + KP * err3 + KD * d3;
    int s2 =  speed + KP * err2 + KD * d2;
    int s4 = speed + KP * err4 + KD * d4;
    
    setMotors(constrain(s1, -255, 255), constrain(s2, -255, 255), constrain(s3, -255, 255), constrain(s4, -255, 255));
    
    errold1 = err1; errold2 = err2; errold3 = err3; errold4 = err4;
    
    Serial.print("df = "); Serial.print(df);
    Serial.print("  enc1="); Serial.print(enc1);
    Serial.print(" enc2="); Serial.print(enc2);
    Serial.print(" enc3="); Serial.print(enc3);
    Serial.print(" enc4="); Serial.println(enc4);
    
    delay(10);
  }
  stopMotors();
}

void setup() {
  pinMode(22, INPUT);
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(25, INPUT);

  pinMode(MOT1_IN1, OUTPUT);
  pinMode(MOT1_IN2, OUTPUT);
  pinMode(MOT2_IN1, OUTPUT);
  pinMode(MOT2_IN2, OUTPUT);
  pinMode(MOT3_IN1, OUTPUT);
  pinMode(MOT3_IN2, OUTPUT);
  pinMode(MOT4_IN1, OUTPUT);
  pinMode(MOT4_IN2, OUTPUT);

  attachInterrupt(0, ENCA, FALLING);
  attachInterrupt(1, ENCB, FALLING);
  attachInterrupt(5, ENCC, FALLING);
  attachInterrupt(4, ENCD, FALLING);

  Serial.begin(115200);
}

void loop() {
  //setMotors(-100, -100, -100, -100);
  turnLeft(700, 200);
  Serial.print(enc1);
  Serial.print(" ");
  Serial.print(enc2);
  Serial.print(" ");
  Serial.print(enc3);
  Serial.print(" ");
  Serial.println(enc4);
  delay(2000);
}
