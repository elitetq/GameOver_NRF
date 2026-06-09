#ifndef TOTS_H
#define TOTS_H

#include <stdbool.h>
#include <inttypes.h>
#include <tots_gl.h>

extern const uint8_t tots_level_1[];


typedef enum {
    TOTS_PLAYER = (uint8_t)0,
    TOTS_PROJECTILE = (uint8_t)1
} tots_entity_type;

typedef enum {
    ANIM_NONE = (uint8_t)0,
    ANIM_LERP = (uint8_t)1
} tots_anim_type;


typedef struct {
    uint16_t x, y, move_x, move_y;
    uint16_t prev_x, prev_y;
    uint16_t tag;
    uint32_t internal_ticks, ticks_per_move;

    // uint16_t anim_timer;
    // uint8_t anim_value;
    void* entity_data;
    tots_entity_type type;
    j_component* sprite;
    // tots_anim_type anim_type;
    bool dirty, anim;
} tots_entity;

void tots_init();

tots_entity* add_entity(tots_entity_type type, void* data, uint16_t tag, int x, int y);
void draw_entities();
int remove_entities(uint8_t tag);


int draw_guy(int x, int y);
int move_entity(tots_entity* entity, int x, int y);

int draw_level(uint8_t* level_dat);

int game_loop();
int update_game();

#endif