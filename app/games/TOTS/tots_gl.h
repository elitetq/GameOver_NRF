#ifndef TOTS_GL_T
#define TOTS_GL_T
#include <J_GL.h>
#include <stdbool.h>
#include <inttypes.h>

static j_decal_data border_decal = {.animation_dat = NULL, .bg_col = PINK, .col = BLACK};
static j_component *border_left, *border_right, *border_top, *border_bottom;
static bool init_game = false;

/*********************************************
                    TAGS
 *********************************************/
#define GUI_TAG 400


int init_graphics();

int draw_borders(bool FILL_MIDDLE);

#endif