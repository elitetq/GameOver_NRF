#ifndef TOTS_H
#define TOTS_H

#include <stdbool.h>
#include <inttypes.h>

extern const uint8_t tots_level_1[];

typedef enum {
    TOTS_PLAYER = (uint8_t)0
} tots_entity_type;

typedef struct {
    uint16_t x, y;
    uint16_t prev_x, prev_y;
    tots_entity_type type;

} tots_entity;

void tots_init(void);

int draw_guy(int x, int y);

int draw_level(uint8_t* level_dat);

#endif