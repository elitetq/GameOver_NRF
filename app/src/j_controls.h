#ifndef J_CONTROLS_T
#define J_CONTROLS_T

#include <inttypes.h>

typedef enum {
    J_SWIPE_NONE = (uint8_t)0x00,
    J_SWIPE_RIGHT = (uint8_t)0x01,
    J_SWIPE_LEFT = (uint8_t)0x02,
    J_SWIPE_UP = (uint8_t)0x03,
    J_SWIPE_DOWN = (uint8_t)0x04
} swipe_dir;

swipe_dir get_swipe_touch(int timeout);

#endif