#include <j_controls.h>


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

swipe_dir get_swipe_touch_async(void) {
    static uint16_t sx = 0, sy = 0;
    static uint8_t cooldown = 0;
    static bool touching = false;

    uint16_t x, y;
    int dx, dy;

    poll_touch_timeout(&x,&y,SWIPE_SAMPLE_MS);
    if(x == 0xFFFF) {
        touching = false;
        return J_SWIPE_NONE;
    }

    if(cooldown) {
        cooldown--;
        return J_SWIPE_NONE;
    }

    if(!touching) {
        touching = true;
        sx = x;
        sy = y;
        return J_SWIPE_NONE; // return since dx and dy will always be 0
    }

    dx = (int)x - (int)sx; dy = (int)y - (int)sy;
    int xabs = abs(dx), yabs = abs(dy);

    if((xabs < SWIPE_MIN_DIST) && (yabs < SWIPE_MIN_DIST)) return J_SWIPE_NONE;

    cooldown = SWIPE_COOLDOWN;
    // Continuous dragging
    sx = x; 
    sy = y;
    
    if(xabs > yabs) return dx > 0 ? J_SWIPE_RIGHT : J_SWIPE_LEFT;
    return dy > 0 ? J_SWIPE_DOWN : J_SWIPE_UP;
}

// Records a finger drag and reports its dominant direction.
swipe_dir get_swipe_touch(int timeout) {
    uint16_t xs[SWIPE_MAX_SAMPLES];
    uint16_t ys[SWIPE_MAX_SAMPLES];
    int count = 0;
    uint16_t x, y;

    // polling touch initially
    poll_touch_timeout(&x, &y, timeout);
    if (x == 0xFFFF) {
        return J_SWIPE_NONE; // no touch within the timeout
    }
    xs[count] = x;
    ys[count] = y;
    count++;

    // sampling while finger is held down
    while (count < SWIPE_MAX_SAMPLES) {
        poll_touch_timeout(&x, &y, SWIPE_SAMPLE_MS);
        if (x == 0xFFFF) {
            break;
        }
        xs[count] = x;
        ys[count] = y;
        count++;
    }

    // net displacement
    int dx = (int)xs[count - 1] - (int)xs[0];
    int dy = (int)ys[count - 1] - (int)ys[0];

    // reject taps or tiny jitters
    if (abs(dx) < SWIPE_MIN_DIST && abs(dy) < SWIPE_MIN_DIST) {
        return J_SWIPE_NONE;
    }

    if (abs(dx) > abs(dy)) {
        return dx > 0 ? J_SWIPE_RIGHT : J_SWIPE_LEFT;
    }
    return dy > 0 ? J_SWIPE_DOWN : J_SWIPE_UP;
}
