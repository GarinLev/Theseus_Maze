void MotorInit(int pinA, int pinB) {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
}

void MotorSet(int speed, int pinA, int pinB) {
  speed = constrain(speed, -255, 255);
  
  if(speed == 0) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, HIGH);
  } else if (speed > 0){
    digitalWrite(pinA, LOW);
    analogWrite(pinB, speed);
  } else if (speed < 0) {
    digitalWrite(pinA, HIGH);
    analogWrite(pinB, speed);
  }
}

void SetAll(int speed) {
  MotorSet(-speed, 4, 5);
  MotorSet(speed, 6, 7);
  MotorSet(-speed, 8, 9);
  MotorSet(speed, 10, 12);
}

void setup() {
  MotorInit(4, 5);
  MotorInit(6, 7);
  MotorInit(8, 9);
  MotorInit(10, 12);
}

int speed = -255;
bool flag = true;
void loop() {
//  if(flag) {
//    speed += 10;
//  } else {
//    speed -= 10;
//  }
//  if(speed < -255)
//    flag = true;
//  if(speed > 255)
//    flag = false;
//

  SetAll(255);
}
