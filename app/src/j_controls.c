#include <j_controls.h>
#include <J_GL.h>
#include <stdlib.h> // abs

#define SWIPE_MAX_SAMPLES 64 // most points recorded in a single swipe
#define SWIPE_SAMPLE_MS   40 // per-sample poll window while the finger is down
#define SWIPE_MIN_DIST    40 // min net travel (px) to count as a swipe vs a tap

// Records a finger drag and reports its dominant direction.
// `timeout` is how long (ms) to wait for the swipe to START before giving up.
swipe_dir get_swipe_touch(int timeout) {
    uint16_t xs[SWIPE_MAX_SAMPLES];
    uint16_t ys[SWIPE_MAX_SAMPLES];
    int count = 0;
    uint16_t x, y;

    // 1) Wait for the swipe to begin, spending the caller's timeout budget here.
    poll_touch_timeout(&x, &y, timeout);
    if (x == 0xFFFF) {
        return J_SWIPE_NONE; // no touch within the timeout
    }
    xs[count] = x;
    ys[count] = y;
    count++;

    // 2) Keep sampling while the finger stays down. A timed-out sample (0xFFFF)
    //    means the finger lifted, so the swipe is over.
    while (count < SWIPE_MAX_SAMPLES) {
        poll_touch_timeout(&x, &y, SWIPE_SAMPLE_MS);
        if (x == 0xFFFF) {
            break;
        }
        xs[count] = x;
        ys[count] = y;
        count++;
    }

    // 3) Net displacement from first to last recorded point.
    int dx = (int)xs[count - 1] - (int)xs[0];
    int dy = (int)ys[count - 1] - (int)ys[0];

    // 4) Reject taps / tiny jitter.
    if (abs(dx) < SWIPE_MIN_DIST && abs(dy) < SWIPE_MIN_DIST) {
        return J_SWIPE_NONE;
    }

    // 5) Whichever axis moved more decides the direction.
    if (abs(dx) > abs(dy)) {
        return dx > 0 ? J_SWIPE_RIGHT : J_SWIPE_LEFT;
    }
    return dy > 0 ? J_SWIPE_DOWN : J_SWIPE_UP;
}
