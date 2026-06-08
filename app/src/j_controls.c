#include <j_controls.h>


static uint16_t xs[SWIPE_MAX_SAMPLES], ys[SWIPE_MAX_SAMPLES];

int store_touch(uint16_t* xs, uint16_t* ys) {
    uint16_t x, y;
    poll_touch_timeout(&x,&y,1);
    if(x == 0xFFFF) return 1;
    xs[0] = x;
    ys[0] = y;
    return 0;
}

int print_direction(swipe_dir dir) {
    switch(dir) {
        case J_SWIPE_DOWN: 
            printk("Swiped down!\n");
            break;
        case J_SWIPE_UP:
            printk("Swiped up!\n");
            break;
        case J_SWIPE_LEFT:
            printk("Swiped left!\n");
            break;
        case J_SWIPE_RIGHT: 
            printk("Swiped right!\n");
            break;
        default:
            break;
    }
    return 0;
}

swipe_dir get_dir(uint16_t* xs, uint16_t* ys, size_t size) {
    int dx = (int)xs[0] - (int)xs[size-1];
    int dy = (int)ys[0] - (int)ys[size-1];

    if(abs(dx) < SWIPE_MIN_DIST && abs(dy) < SWIPE_MIN_DIST) return J_SWIPE_NONE;

    if((float)abs(dx) > Y_WEIGHT * (float)abs(dy)) return dx > 0 ? J_SWIPE_LEFT : J_SWIPE_RIGHT;

    return dy > 0 ? J_SWIPE_UP : J_SWIPE_DOWN;
}

swipe_dir get_swipe_touch_async() {
    static int i = 0;
    int ret;

    ret = store_touch(xs + i,ys + i);
    if(i == SWIPE_MAX_SAMPLES_ASYNC - 1) {
        i = 0;
        return get_dir(xs, ys, SWIPE_MAX_SAMPLES_ASYNC);
    } 
    else if(ret) i = 0;
    else i++;

    return J_SWIPE_NONE;
}

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
