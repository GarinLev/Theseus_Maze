#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <VL53L0X.h>
#include <Servo.h>
#include <microLED.h>

#define MOT1_IN1 6
#define MOT1_IN2 7
#define MOT2_IN1 8
#define MOT2_IN2 9
#define MOT3_IN1 10
#define MOT3_IN2 11
#define MOT4_IN1 12
#define MOT4_IN2 4

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

#define CAL_BUTTON 42
#define COLOR_THRESHOLD 10000

volatile long encoder1 = 0;
volatile long encoder2 = 0;
volatile long encoder3 = 0;
volatile long encoder4 = 0;

#define NUM_SENSORS 6
VL53L0X lox[NUM_SENSORS];
int16_t lox_adresses[NUM_SENSORS] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 };
int16_t lox_shts[NUM_SENSORS] = { 32, 33, 34, 14, 15, 16 };

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint16_t black_r, black_g, black_b;
uint16_t red_r, red_g, red_b;
uint16_t blue_r, blue_g, blue_b;

Servo myServo;

void setup() {
  Serial.begin(115200);
  Wire.begin();        
  myServo.attach(5);// обязательно!
  setupMotors();
  setupVLX();              // теперь настраиваем лазеры
  setupTCS();              // затем датчик цвета (адрес 0x29 свободен)
  pinMode(CAL_BUTTON, INPUT_PULLUP);
  calibrateColors();
  Serial.println(get_distance(0));
  myServo.write(0); 
}



void rightArm(){
  bool new_square = false;
  while (!new_square) {
    if (checkRight()) {
      if (checkForward()) {
        setMotors(150, 150, 150, 150);
        delay(200);
        stopMotors;
        delay(400);
        moveBack(15, 200);
        stopMotors;
        delay(400);
        turnLeft(1400, 200);
      }
      else {
        moveForward(600, 200);
        if (checkForward()) {
          moveBack(15, 200);
          stopMotors;
          delay(400);
        }
        new_square = true;
      }
    }
    else {
      turnRight(1400, 200);
      moveForward(600, 200);
      if (get_distance(0) < 60 && get_distance(0) != 0) {
        moveBack(25, 200);
        stopMotors;
        delay(400);
      }
      new_square = true;
    }
  }
}


void leftArm(){
  bool new_square = false;
  while (!new_square) {
  if (checkLeft()) {
    if (checkForward()) {
      setMotors(150, 150, 150, 150);
      delay(200);
      stopMotors;
      delay(400);
      moveBack(15, 200);
      stopMotors;
      delay(400);
      turnRight(1400, 200);  
    }
    else {                     
      moveForward(600, 200);
      if (checkForward()) {
        moveBack(15, 200);
        stopMotors;
        delay(400);
      }
      new_square = true;
    }
  }
  else {                      
    turnLeft(1400, 200);
    moveForward(600, 200);
    if (get_distance(0) < 60 && get_distance(0) != 0) {
      moveBack(25, 200);
      stopMotors;
      delay(400);
    }
    new_square = true;
  }
}
}


void loop() {
  int squares = 0;
  while(squares < 50){ 
    rightArm();
    squares++;
  }
  while(!checkLeft){
    rightArm();
  }
  while(){
    leftArm();
  }
}
