#pragma once

#include "../soft_move.h"

struct RotateController {
	struct pt pt_task;
	AngelNode angel;
	SoftMove profile;

	enum States {
		STOP,
		RUN
	};
	States state = STOP;

	void init() {
		PT_INIT(&pt_task);

		angel.dt = 11;

		node_angel_init(angel);
	}

	void update() {
		if (state == RUN) {
			
		}
	}


	void setMove(float start_pos, float total_dist, float acc_dist, float target_v) {
		
	}
};