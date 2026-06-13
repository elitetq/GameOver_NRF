#ifndef TOTS_GL_T
#define TOTS_GL_T
#include <J_GL.h>
#include <stdbool.h>

#include <inttypes.h>

#define GRID_SIZE 28

#define PLAYER_COLOR YELLOW

extern j_decal_data border_decal, player_decal, empty_decal, enemy_decal, brick_decal, white_decal, highlight_decal, player_trail_decal;
extern j_component *border_left, *border_right, *border_top, *border_bottom, *brick_comp, *black_square, *white_square, *player_square, *point_square;

extern j_component *char_sprite;


extern bool init_game;

/*********************************************
                    TAGS
 *********************************************/
#define GUI_TAG 400
#define PLAYER_TAG 450
#define BRICK_TAG 500
#define ENEMY_TAG 550


int init_graphics();

int draw_borders(bool FILL_MIDDLE);

int draw_sprite(int16_t x, int16_t y, j_component* sprite_comp);

int tots_draw_points(uint16_t points);
#endif