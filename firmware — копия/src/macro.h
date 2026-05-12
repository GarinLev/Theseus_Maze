#pragma once

#include <stdint.h>

typedef uint8_t seq_t;

#define TOPIC_FIELDS seq_t seq
#define SUB_FIELDS   seq_t last_seq

#define HAS_NEW_DATA(node, topic) (topic.seq != node.last_seq)
#define SYNC_TOPIC(node, topic)   (node.last_seq = topic.seq)
#define WAIT_NEW_DATA(node) \
    PT_WAIT_UNTIL(&(node).pt, HAS_NEW_DATA((node), (*(node).topic)))

#define SUBSCRIBE_TOPIC(node, target_topic) \
    do { \
        (node).topic = (target_topic); \
        SYNC_TOPIC((node), (*(target_topic))); \
    } while(0)

#define NOTIFY_TOPIC(topic_ptr) ((topic_ptr)->seq++)

#define PIN_NONE 255