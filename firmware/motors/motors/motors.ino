#define MOTOR_A_IN1 4
#define MOTOR_A_IN2 5
#define MOTOR_B_IN1 6
#define MOTOR_B_IN2 7

#define ENCODER_A_PIN_A 18
#define ENCODER_A_PIN_B 19
#define ENCODER_B_PIN_A 2
#define ENCODER_B_PIN_B 3

#define KP 2.0
#define KI 0.05
#define KD 0.1
#define MAX_PWM 255

volatile long encoderA = 0;
volatile long encoderB = 0;

float lastError = 0;
float integral = 0;
unsigned long lastTime = 0;

void setupPins() {
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  
  pinMode(ENCODER_A_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_A_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN_B, INPUT_PULLUP);
}

void encoderA() {
  if (digitalRead(ENCODER_A_PIN_B) == HIGH) {
    encoderA++;
  } else {
    encoderA--;
  }
}

void encoderB() {
  if (digitalRead(ENCODER_B_PIN_B) == HIGH) {
    encoderB++;
  } else {
    encoderB--;
  }
}

void setMotorA(int pwm) {
  pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
  
  if (pwm >= 0) {
    analogWrite(MOTOR_A_IN1, pwm);
    analogWrite(MOTOR_A_IN2, 0);
  } else {
    analogWrite(MOTOR_A_IN1, 0);
    analogWrite(MOTOR_A_IN2, abs(pwm));
  }
}

void setMotorB(int pwm) {
  pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
  
  if (pwm >= 0) {
    analogWrite(MOTOR_B_IN1, pwm);
    analogWrite(MOTOR_B_IN2, 0);
  } else {
    analogWrite(MOTOR_B_IN1, 0);
    analogWrite(MOTOR_B_IN2, abs(pwm));
  }
}

void moveForwardSync(int speed) {
  long diff = encoderA - encoderB;
  float error = diff;
  
  unsigned long timeNow = millis();
  float deltaTime = (timeNow - lastTime) / 1000.0;
  lastTime = timeNow;
  
  integral += error * deltaTime;
  float derivative = (error - lastError) / deltaTime;
  lastError = error;
  
  float correction = KP * error + KI * integral + KD * derivative;
  correction = constrain(correction, -50, 50);
  
  setMotorA(speed - correction);
  setMotorB(speed + correction);
}

void turnSync(int speed, float turnFactor) {
  long diff = encoderA - encoderB;
  float targetDiff = turnFactor * 50;
  float error = diff - targetDiff;
  
  unsigned long timeNow = millis();
  float deltaTime = (timeNow - lastTime) / 1000.0;
  lastTime = timeNow;
  
  integral += error * deltaTime;
  float derivative = (error - lastError) / deltaTime;
  lastError = error;
  
  float correction = KP * error + KI * integral + KD * derivative;
  correction = constrain(correction, -50, 50);
  
  setMotorA(speed - correction);
  setMotorB(speed + correction);
}

void rotateInPlace(int speed, bool clockwise) {
  if (clockwise) {
    setMotorA(speed);
    setMotorB(-speed);
  } else {
    setMotorA(-speed);
    setMotorB(speed);
  }
}

void resetEncoders() {
  encoderA = 0;
  encoderB = 0;
  lastError = 0;
  integral = 0;
}

void stopMotors() {
  setMotorA(0);
  setMotorB(0);
}

void setup() {
  Serial.begin(115200);
  setupPins();
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN_A), encoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN_A), encoderB, RISING);
  
  lastTime = millis();
}

void loop() {
  moveForwardSync(100);
  delay(10);
}
