#include "robot.h"

namespace robot {
    WallController wallLeft;
    //WallController wallRight;

    WheelController wheelA1;
    WheelController wheelA2;
    WheelController wheelB1;
    WheelController wheelB2;

    static void initWheel() {
        int dist = 500;
        wheelA1.init(500.0f, 140.0f);
        wheelA1.setPins(4, 5, false, 2, 22, robot::isr_encoder_A1);
        wheelA1.goTo(100, (dist / 2) - 10, dist);

        wheelA2.init(500.0f, 140.0f);
        wheelA2.setPins(6, 7, false, 18, 24, robot::isr_encoder_A2);
        wheelA2.goTo(100, (dist / 2) - 10, dist);

        wheelB1.init(500.0f, 140.0f);
        wheelB1.setPins(8, 9, true, 3, 23, robot::isr_encoder_B1);
        wheelB1.goTo(100, (dist / 2) - 10, dist);

        wheelB2.init(500.0f, 140.0f);
        wheelB2.setPins(10, 12, true, 19, 25, robot::isr_encoder_B2);
        wheelB2.goTo(100, (dist / 2) - 10, dist);
    }

    static void initWall() {

    }
    
    void init() {
        initWall();
        initWall();
    }
}
