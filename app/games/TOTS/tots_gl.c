#include <tots_gl.h>
#include <tots_resource.h>
#include <J_GL.h>


int init_graphics() {
    if(init_game) return;

    border_left = create_component_t(GUI_TAG,"tots_border_left", J_DECAL, 0, 0, tots_border_left, &border_decal);
    border_right = create_component_t(GUI_TAG,"tots_border_right", J_DECAL, 240-22, 0, tots_border_right, &border_decal);
    border_top = create_component_t(GUI_TAG,"tots_border_top", J_DECAL, 22, 0, tots_border_top, &border_decal);
    border_bottom = create_component_t(GUI_TAG,"tots_border_bottom", J_DECAL, 22, 300, tots_border_bottom, &border_decal);

    init_game = true;
}

int draw_borders(bool FILL_MIDDLE) {
    draw_component(border_left);
    draw_component(border_right);
    draw_component(border_top);
    draw_component(border_bottom);

    if(!FILL_MIDDLE) return 0;

    j_color T_BG_COLOR = BLACK;
    j_shape_data shape_dat = {
        .type=J_RECTANGLE,
        .bg_col=BLACK,
        .col=BLACK,
        .centering=J_LEFT,
        .height=280,
        .length=196
    };
    j_component* T_BG_BLACK = create_component("black_bg",J_SHAPE,22,20,(void*)&shape_dat,NULL);
    draw_component(T_BG_BLACK);
    free_component(T_BG_BLACK);


    return 0;
}