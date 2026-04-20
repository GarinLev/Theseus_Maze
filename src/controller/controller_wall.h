#pragma once

#include "../node/nodes.h"
#include "../../lib/pt/pt.h"

struct WallController {
	DistNode nodeUp, nodeWall;
	PI_Node wallNode; PI_Topic wallTopic;

	struct pt pt_task;
	uint32_t  last_time = 0;

	uint16_t offset = 0;

	void init() {
		wallNode.topic = &wallTopic;

		wallNode.Kp = 1;
		wallNode.Ki = 0;
		wallNode.dt = 25;

		PT_INIT(&pt_task);
	}

	void setPins(TwoWire* wire, uint8_t addrUp, uint8_t addrWall,
		uint8_t pinShtUp, uint8_t pinShtWall) {
		nodeUp.wire = wire;
		nodeUp.addr = addrUp;
		nodeUp.pin_sht = pinShtUp;

		nodeWall.wire = wire;
		nodeWall.addr = addrWall;
		nodeWall.pin_sht = pinShtWall;

		node_dist_init(nodeUp);
		node_dist_init(nodeWall);
	}

	uint16_t l1 = 63, l2 = 55;
	static constexpr float COS45 = 0.70710678118;

	int task_process() {
		PT_BEGIN(&pt_task);

		HERE:
		{
			PT_WAIT_UNTIL(&pt_task, (uint32_t)(millis() - last_time) >= 25);
			last_time = millis();

			wallNode.setpoint = 0;
			
			uint16_t d1 = nodeWall.dist_out;
			uint16_t d2 = nodeUp.dist_out;
			uint16_t d1x = l1 + d1;
			uint16_t d2x = l2 + COS45 * d2;
			int16_t error = d1x - d2x;

			wallTopic.value = error;
			NOTIFY_TOPIC(&wallTopic);

			offset = wallNode.value_out;
			goto HERE;
		}

		PT_END(&pt_task);
	}

	void update() {
		node_pi_run(wallNode);
		node_dist_run(nodeUp);
		node_dist_run(nodeWall);
	}
};