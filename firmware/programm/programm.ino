#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_TCS34725.h>

#define MOT1_IN1 6
#define MOT1_IN2 7
#define MOT2_IN1 8
#define MOT2_IN2 9
#define MOT3_IN1 10
#define MOT3_IN2 11
#define MOT4_IN1 12
#define MOT4_IN2 13

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

const uint8_t xshutPins[] = {32, 33, 34, 14, 15, 16};
const int numSensors = sizeof(xshutPins) / sizeof(xshutPins[0]);
Adafruit_VL53L0X sensor;
uint8_t sensorAddresses[numSensors];
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint16_t black_r, black_g, black_b;
uint16_t red_r, red_g, red_b;
uint16_t blue_r, blue_g, blue_b;

void setup() {
  Serial.begin(115200);
  setupMotors();
  setupVLX();
  setupTCS();
  pinMode(CAL_BUTTON, INPUT_PULLUP);
  calibrateColors();
}

void loop() {
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
