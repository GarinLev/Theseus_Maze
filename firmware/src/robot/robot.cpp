#include "robot.h"
#include <Wire.h>
#include "../controller/controller_com.h"

namespace robot {
    TaskRobot task = TaskRobot_WAIT;
    StateRobot state = StateRobot_WAIT;

    WallManager wallManager;
    WallController wallRight, wallLeft;
    WheelManager wheelManager;
    WheelController wheelA1, wheelA2, wheelB1, wheelB2;
    DebugController debug;
    RotateController rotateController;
    ServoController servoController;
    LightController lightController;
    ColorController colorController;

    const int16_t OFFSETS_MPU[6] PROGMEM = {
        -2184, -3266, 2904, -1830, -66, -17
    };

    const float OFFSETS_COLOR[8] PROGMEM = {
        1.21f, 0.69f, 1.39f, // Баланс белого
        (190.5f / 100.0f), // Синиий цвет
        756.0f, 16.70f,
        385.0f, 50.46f
    };

#ifdef COM_ENABLE
    ComController comController;
#endif 

    static void sentData();
    static void waitForSensors(uint32_t timeout_ms);
    void calibrate();

    void init() {
        Serial.begin(115200);
        Serial2.begin(9600);
        Wire.begin();
        Wire.setClock(400000);

        wheelA1.init(500, 140); wheelA1.setPins(4, 5, false, 2, 22, robot::encoder::isr_A1);
        wheelA2.init(500, 140); wheelA2.setPins(8, 9, false, 18, 24, robot::encoder::isr_A2);
        wheelB1.init(500, 140); wheelB1.setPins(6, 7, true, 3, 23, robot::encoder::isr_B1);
        wheelB2.init(500, 140); wheelB2.setPins(10, 12, true, 19, 25, robot::encoder::isr_B2);

        wheelManager.fr = &wheelA1; wheelManager.fl = &wheelA2;
        wheelManager.br = &wheelB1; wheelManager.bl = &wheelB2;
        wheelManager.init();
        wheelManager.setDistSource(&wallRight.nodeExtra);
        wheelManager.setPitchSource(&rotateController.angel.ypr[1]);

        wallRight.setPins(&Wire, 0x30, 0x31, 0x32, 37, 36, 32);
        wallLeft.setPins(&Wire, 0x33, 0x34, 0x35, 33, 34, 35);
        wallRight.reset(); wallLeft.reset();
        delay(100);

        colorController.init();
        delay(50);

        wallRight.setAddr(); wallLeft.setAddr();
        wallRight.init(); wallLeft.init();

        wallManager.sens_left = &wallLeft;
        wallManager.sens_right = &wallRight;
        wallManager.wheelA1 = &wheelA1; wallManager.wheelA2 = &wheelA2;
        wallManager.wheelB1 = &wheelB1; wallManager.wheelB2 = &wheelB2;
        wallManager.init();

        rotateController.wheelA1 = &wheelA1; rotateController.wheelA2 = &wheelA2;
        rotateController.wheelB1 = &wheelB1; rotateController.wheelB2 = &wheelB2;
        rotateController.init();

        servoController.pin = 44;
        servoController.init();

        pinMode(42, INPUT_PULLUP);
        if (digitalRead(42) == LOW) {
            calibrate();
        }


        Serial.print("Waiting...   ");
#ifdef COM_ENABLE
        comController.init();
        for (;;) {
            comController.node.request[0] = 0;
            node_com_run(comController.node);
            if (comController.node.command == 's') break;
            comController.node.request[0] = 's';
            node_com_run(comController.node);
            delay(100);
        }
        waitForSensors(800);
        sentData();
#endif
        Serial.println("Done");
    }
   
    void loop() {
        debug.frame();
        debug.update();
        debug.ups_update();

#ifdef COM_ENABLE
        if (state != StateRobot_VICTIM)
        {
            comController.update(&task);
        }
#endif

        static bool last_btn = HIGH;
        bool current_btn = digitalRead(42);
        if (current_btn == LOW && last_btn == HIGH) {

            if (task != TaskRobot_END) {
                comController.node.request[0] = 'p';
                task = TaskRobot_END;
            }
            else {
                comController.node.request[0] = 's';
                task = TaskRobot_WAIT;
            }
            node_com_run(comController.node);
        }
        last_btn = current_btn;

        node_dist_run(wallRight.nodeWall);
        node_dist_run(wallLeft.nodeWall);
        node_dist_run(wallRight.nodeExtra);
        node_dist_run(wallLeft.nodeExtra);

        if (task >= TaskRobot_VICTIM_LEFT && task <= TaskRobot_VICTIM_RIGHT_X2) {
            update_victim();
        }
        else if (task >= TaskRobot_STEP_UP && task <= TaskRobot_STEP_DOWN) {
            update_step();
        }
        else {
            state = StateRobot_WAIT;
        }

        if (state == StateRobot_WAIT) handleWait();
        else if (state == StateRobot_MOVE) handleMove();
        else if (state == StateRobot_ROTATE) handleRotate();
        else if (state == StateRobot_VICTIM) handleVictim();

        lightController.update();
        wheelManager.update();
    }



       
    static TaskRobot task_step;
    void update_step() {
        static uint16_t step_local_state = 0;
        static uint16_t step_timer = 0;
        
        if (step_local_state == 0) {
            Serial.println("s0");
            task_step = task;
            if (task == TaskRobot_STEP_UP) {
                step_local_state = 3;
            }
            else {
                state = StateRobot_ROTATE;
                if (task == TaskRobot_STEP_DOWN) rotateController.run(175, 100);
                else if (task == TaskRobot_STEP_RIGHT) rotateController.run(85, 100);
                else if (task == TaskRobot_STEP_LEFT) rotateController.run(-85, 100);
                step_local_state = 1;
            }
        }
        else if (step_local_state == 1) {
            if (rotateController.is_active) {
                state = StateRobot_ROTATE;
                return;
            }
            Serial.println("s1");
            step_timer = millis();
            step_local_state = 2;
        }
        else if (step_local_state == 2) {
            if (millis() - step_timer < 200) return;
            Serial.println("s2");
            step_local_state = 3;
        }
        else if (step_local_state == 3) {
            Serial.println("s3");
            state = StateRobot_MOVE;
            if (!wheelManager.is_moving) {
                wheelManager.moveDistance(310.0f, 85.0f, &rotateController.angel.ypr[1]);
            }
            step_local_state = 4;
        }
        else if (step_local_state == 4) {
            if (wheelManager.is_moving) {
                state = StateRobot_MOVE;
                return;
            }
            Serial.println("s4");
            sentData();
            task_step = TaskRobot_WAIT;
            task = TaskRobot_WAIT;
            step_local_state = 0;
        }
    }

    void update_victim() {
        bool isLeft = (task == TaskRobot_VICTIM_LEFT || task == TaskRobot_VICTIM_LEFT_X2);
        bool isX2 = (task == TaskRobot_VICTIM_LEFT_X2 || task == TaskRobot_VICTIM_RIGHT_X2);
        static uint16_t victim_local_state = 0;


        if (victim_local_state == 0) {
            Serial.println("v0");
            state = StateRobot_VICTIM;
            servoController.set(isLeft ? ServoController_Left : ServoController_Right);
            victim_local_state = 1;
        }
        else if (victim_local_state == 1) {
            if (servoController.is_active) return;
            Serial.println("v1");
            victim_local_state = 2;
        }
        else if (victim_local_state == 2) {
            servoController.set(isLeft ? ServoController_CloseLeft : ServoController_CloseRight);
            Serial.println("v2");
            victim_local_state = 3;
        }
        else if (victim_local_state == 3) {
            if (servoController.is_active) return;
            Serial.println("v3");
            victim_local_state = isX2 ? 4 : 8;
        }
        else if (victim_local_state == 4) {
            servoController.set(isLeft ? ServoController_Left : ServoController_Right);
            Serial.println("v4");
            victim_local_state = 5;
        }
        else if (victim_local_state == 5) {
            if (servoController.is_active) return;
            Serial.println("v5");
            victim_local_state = 6;
        }
        else if (victim_local_state == 6) {
            Serial.println("v6");
            servoController.set(isLeft ? ServoController_CloseLeft : ServoController_CloseRight);
            victim_local_state = 7;
        }
        else if (victim_local_state == 7) {
            if (servoController.is_active) return;
            Serial.println("v7");
            victim_local_state = 8;
        }
        else if (victim_local_state == 8) {
            Serial.println("v8");
            task = task_step;
            state = StateRobot_WAIT;
            victim_local_state = 0;
        }
    }


    

    static void waitForSensors(uint32_t timeout_ms) {
        uint32_t start = millis();
        while (millis() - start < timeout_ms) {
            node_dist_run(wallRight.nodeUp);
            node_dist_run(wallRight.nodeWall);
            node_dist_run(wallRight.nodeExtra);
            node_dist_run(wallLeft.nodeUp);
            node_dist_run(wallLeft.nodeWall);
            node_dist_run(wallLeft.nodeExtra);
            delay(5);
        }
    }

    static void sentData() {
#ifdef COM_ENABLE
        uint16_t distUp = wallRight.nodeExtra.dist_valid ? wallRight.nodeExtra.dist : 0;
        uint16_t distLeft = wallLeft.nodeWall.dist_valid ? wallLeft.nodeWall.dist : 0;
        uint16_t distDown = wallLeft.nodeExtra.dist_valid ? wallLeft.nodeExtra.dist : 0;
        uint16_t distRight = wallRight.nodeWall.dist_valid ? wallRight.nodeWall.dist : 0;

        uint16_t walls[4] = { distUp, distLeft, distDown, distRight };
        uint8_t step_count = wheelManager.hasDouble ? 2 : 1;
        wheelManager.hasDouble = false;
        comController.sentData(walls, false, step_count);
#endif
    }

    void calibrate() {
        Serial.println(F("Calibrate Mode..."));

        auto waitClick = [](int pin) {
            while (digitalRead(pin) == HIGH);
            delay(50);

            while (digitalRead(pin) == LOW);
            delay(50);
            }; 

        Serial.println(F("Factor WHITE:  "));
        waitClick(42);
        colorController.calibrateWhite();

        Serial.println(F("Color WHITE:  "));
        waitClick(42);
        colorController.calibrateColor();

        Serial.println(F("Color BLUE:  "));
        waitClick(42);
        colorController.calibrateColor();

        Serial.println(F("Color GREY:  "));
        waitClick(42);
        colorController.calibrateColor();

        Serial.println(F("Color BLACK:  "));
        waitClick(42);
        colorController.calibrateColor();

        Serial.println(F("Calibrate MPU"));
        rotateController.calibrate();
        Serial.println(F("Calibrate end. Please reboot."));
        for (;;);
    }
     
    void handleWait() { wheelManager.stop(); }
    void handleRotate() { rotateController.update(); }
    void handleVictim() { servoController.update(); }
    void handleMove() {
        node_angel_run(rotateController.angel);
        if (!wheelManager.wall_disable) {
            wallRight.update();
            wallLeft.update();
            wallManager.update();
        }
        else {
            wheelA1.speed_offset = wheelA2.speed_offset = 0;
            wheelB1.speed_offset = wheelB2.speed_offset = 0;
        }
        colorController.update();
    }
}