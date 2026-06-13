#include <tots_gl.h>
#include <tots_resource.h>
#include <J_GL.h>


j_text_data points_text_dat = {.bg_col = RED, .col = BLACK, .centering = J_LEFT, .font_size = FONT_MEDIUM};
j_decal_data border_decal = {.animation_dat = NULL, .bg_col = RED, .col = BLACK};
j_decal_data player_decal = {.animation_dat = NULL, .bg_col = BLACK, .col = YELLOW};
j_decal_data highlight_decal = {.animation_dat = NULL, .bg_col = BLACK, .col = WHITE};
j_decal_data brick_decal = {.animation_dat = NULL, .bg_col = BLACK, .col = RED};
j_decal_data enemy_decal = {.animation_dat = NULL, .bg_col = BLACK, .col = BLUE};
j_decal_data empty_decal = {.animation_dat = NULL, .bg_col = BLACK, .col = BLACK};
j_shape_data border_square_shape_data = {.bg_col = BLACK, .col = RED, .centering = J_LEFT, .height = 20, .length = 50, .type = J_RECTANGLE};
j_decal_data white_decal = {.animation_dat = NULL, .bg_col = PLAYER_COLOR, .col = PLAYER_COLOR};
j_decal_data player_trail_decal = {.animation_dat = NULL, .bg_col = PLAYER_COLOR, .col = PLAYER_COLOR};
j_component *border_left, *border_right, *border_top, *border_bottom, *brick_comp, *black_square, *white_square, *player_square, *point_square, *point_text, *border_square;
j_component *char_sprite;
static char points_str[5] = {'0','0','0','0',0};
bool init_game = false;

#define TEXT_X_POS 40
#define TEXT_Y_POS 2

int init_graphics() {
    if(init_game) return 1;

    border_left = create_component_t(GUI_TAG,"tots_border_left", J_DECAL, 0, 0, tots_border_left, &border_decal);
    border_right = create_component_t(GUI_TAG,"tots_border_right", J_DECAL, 240-22, 0, tots_border_right, &border_decal);
    border_top = create_component_t(GUI_TAG,"tots_border_top", J_DECAL, 22, 0, tots_border_top, &border_decal);
    border_bottom = create_component_t(GUI_TAG,"tots_border_bottom", J_DECAL, 22, 300, tots_border_bottom, &border_decal);

    char_sprite = create_component_t(PLAYER_TAG,"char_sprite",J_DECAL,50,50,Guy_0,&player_decal);

    brick_comp = create_component_t(BRICK_TAG,"brick_texture",J_DECAL,0,0,brick_texture,&brick_decal);

    black_square = create_component_t(BRICK_TAG,"no_brick_texture",J_DECAL,0,0,no_texture,&empty_decal);

    white_square = create_component_t(BRICK_TAG,"white_trail",J_DECAL,0,0,brick_texture,&white_decal);

    player_square = create_component_t(BRICK_TAG,"player_trail",J_DECAL,0,0,brick_texture,&player_trail_decal);

    point_square = create_component_t(BRICK_TAG,"point_space",J_DECAL,0,0,point_space,&player_decal);

    point_text = create_component_t(GUI_TAG,"points",J_TEXT,TEXT_X_POS,TEXT_Y_POS,points_str,&points_text_dat);

    border_square = create_component_t(BRICK_TAG,"border_square",J_SHAPE,0,0,(void*)&border_square_shape_data,NULL);

    init_game = true;
    return 0;
}

int tots_draw_points(uint16_t points) {

    static uint16_t points_memory = 0;
    // if(points_memory == points) return 1;
    points_str[3] = '0' + points % 10;
    points_str[2] = '0' + (points / 10) % 10;
    points_str[1] = '0' + (points / 100) % 10;
    points_str[0] = '0' + (points / 1000) % 10;
    draw_component(point_text);

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

int draw_sprite(int16_t x, int16_t y, j_component* sprite_comp) {
    if(x > MAZE_X_LEN || x < 1 || y > MAZE_Y_LEN || y < 1) return 1;
    
    sprite_comp->x = 22 + GRID_SIZE*(x-1);
    sprite_comp->y = 20 + GRID_SIZE*(y-1);
    draw_component(sprite_comp);

    return 0;
}
