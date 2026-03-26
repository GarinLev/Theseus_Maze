#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_TCS34725.h>

#define MOT1_IN1 4
#define MOT1_IN2 5
#define MOT2_IN1 6
#define MOT2_IN2 7
#define MOT3_IN1 8
#define MOT3_IN2 9
#define MOT4_IN1 10
#define MOT4_IN2 13
ы
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
const uint8_t xshutPins[] = {40, 41, 42, 43, 44, 45};
const int numSensors = sizeof(xshutPins);
Adafruit_VL53L0X sensor;
uint8_t sensorAddresses[numSensors];
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS,TCS34725_GAIN_4X);

void setup() {
  Serial.begin(115200);
  setupMotors();
  setupVLX();
}

void loop() {
  bool new_square = false;
  while(!new_square){
    if(checkRight()){
      if(checkForward()){
        turnLeft(1350, 100);
      }
      else{
        moveForward(600, 75);
        new_square = true;
      }
    }
    else{
      turnRight(1400, 100);
      moveForward(600, 100);
      new_square = true;
    }
  }
  
}
