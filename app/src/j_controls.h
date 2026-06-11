#ifndef J_CONTROLS_T
#define J_CONTROLS_T

#include <J_GL.h>
#include <stdlib.h> // abs
#include <inttypes.h>
#include <inttypes.h>

#define SWIPE_MAX_SAMPLES 32 // most points recorded in a single swipe
#define SWIPE_SAMPLE_MS   1 // per-sample poll window while the finger is down
#define SWIPE_MIN_DIST    30 // min net travel (px) to count as a swipe vs a tap

#define SWIPE_COOLDOWN 2 // Cooldown (in game ticks) to wait for next swipe
#define Y_WEIGHT (float)0.8f // Weighting for the Y displacement, its easier to swipe in the y-dir than the x-dir. this makes swiping feel more accurate


// Try changing this and seeing if buggy
typedef enum {
    J_SWIPE_NONE = (uint8_t)0x00,
    J_SWIPE_RIGHT = (uint8_t)0x01,
    J_SWIPE_LEFT = (uint8_t)0x02,
    J_SWIPE_UP = (uint8_t)0x03,
    J_SWIPE_DOWN = (uint8_t)0x04
} swipe_dir;

swipe_dir get_swipe_touch(int timeout);

swipe_dir get_swipe_touch_async(void);

int print_direction(swipe_dir dir);

#endif