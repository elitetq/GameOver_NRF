#include <J_GL.h>
#include <tots_resource.h>
#include <stdbool.h>

static j_decal_data border_decal = {.animation_dat = NULL, .bg_col = PINK, .col = MAGENTA};
static j_component *border_left, *border_right, *border_top, *border_bottom;
static bool init_game = false;

#define GUI_TAG 400

void tots_init(void) {
    if(init_game) return;

    border_left = create_component_t(GUI_TAG,"tots_border_left", J_DECAL, 0, 0, tots_border_left, &border_decal);
    border_right = create_component_t(GUI_TAG,"tots_border_right", J_DECAL, 200, 0, tots_border_right, &border_decal);
    border_top = create_component_t(GUI_TAG,"tots_border_top", J_DECAL, 0, 0, tots_border_top, &border_decal);
    border_bottom = create_component_t(GUI_TAG,"tots_border_bottom", J_DECAL, 0, 280, tots_border_bottom, &border_decal);

    init_game = true;
}

int draw_borders(bool FILL_MIDDLE) {
    draw_component(border_left);
    draw_component(border_right);
    draw_component(border_top);
    draw_component(border_bottom);


    return 0;
}
