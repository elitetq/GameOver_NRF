#include <tots.h>
#include <tots_gl.h>
#include <tots_resource.h>
#include <stdbool.h>
#include <string.h>
#include <j_controls.h>

uint32_t last_frame, now_frame, fps;

#define MAX_ENTITIES 25

static int entity_array_index = 0;
static tots_entity tots_entity_array[MAX_ENTITIES] = {0};

static uint8_t* cur_level;


#define FRAME_PER_SECOND 30
#define FRAME_MS 1000/FRAME_PER_SECOND

#define LEVEL_W    (MAZE_X_LEN * GRID_SIZE)   // 196  full image width
#define LEVEL_H    (MAZE_Y_LEN * GRID_SIZE)   // 280  full image height
#define LEVEL_BODY ((LEVEL_W * LEVEL_H) / 8)  // packed 1bpp, no header


//14 x 20


void tots_init() {
    init_graphics();
    draw_borders(true);
    last_frame = k_uptime_get_32();
    now_frame = k_uptime_get_32();
    cur_level = tots_level_1;
    draw_level(cur_level);
    tots_entity *plr = add_entity(TOTS_PLAYER,PLAYER_TAG,1,1);
    if(plr != NULL) {
        plr->move_x = 5;
        plr->move_y = 1;
    }
}

int draw_guy(int x, int y) {
    return draw_sprite(x,y,char_sprite);
}


int draw_level(uint8_t* level_dat) {
    static uint8_t level_dat_comb[LEVEL_BODY + 4]; // in BSS, no heap needed

    // 4-byte decal header: [height_hi, height_lo, length_hi, length_lo]
    level_dat_comb[0] = (LEVEL_H >> 8) & 0xFF;
    level_dat_comb[1] = LEVEL_H & 0xFF;
    level_dat_comb[2] = (LEVEL_W >> 8) & 0xFF;
    level_dat_comb[3] = LEVEL_W & 0xFF;

    memset(level_dat_comb + 4, 0x00, LEVEL_BODY);

    for(size_t cell = 0; cell < MAZE_X_LEN * MAZE_Y_LEN; cell++) {
        // Reverse level array
        size_t local_cell = MAZE_X_LEN - 1 - cell + (2*MAZE_X_LEN)*(cell/MAZE_X_LEN);
        const uint8_t* tex = (level_dat[local_cell] == 1 ? brick_texture : no_texture) + 4; // skip header
        size_t x0 = (cell % MAZE_X_LEN) * GRID_SIZE; // cell's left edge in full image
        size_t y0 = (cell / MAZE_X_LEN) * GRID_SIZE; // cell's top edge in full image

        // Copy the cell bit-by-bit into its (x,y) slot. 28px isn't byte-aligned,
        // so we can't memcpy rows; each pixel lands at bit (Y*LEVEL_W + X).
        for(size_t cy = 0; cy < GRID_SIZE; cy++) {
            for(size_t cx = 0; cx < GRID_SIZE; cx++) {
                size_t sbit = cy * GRID_SIZE + cx;                  // bit within the cell
                if(!(tex[sbit >> 3] & (0x80 >> (sbit & 7)))) continue;
                size_t dbit = (y0 + cy) * LEVEL_W + (x0 + cx);      // bit within full image
                level_dat_comb[4 + (dbit >> 3)] |= (0x80 >> (dbit & 7));
            }
        }
    }

    j_decal_data level_decal_dat = {.animation_dat=NULL,.bg_col=brick_decal.bg_col,.col=brick_decal.col};
    j_component* level_comp = create_component("level_comp",J_DECAL,22,20,(void*)level_dat_comb,&level_decal_dat);
    draw_component(level_comp);
    free_component(level_comp);   // frees the j_component struct (heap); buffer is static

    return 0;
}

int game_loop() {
    now_frame = k_uptime_get_32();

    if((now_frame - last_frame) >= FRAME_MS) {
        printk("FPS: %d\n", fps);
        fps = (now_frame - last_frame) ? 1000/(now_frame-last_frame) : fps;
        update_game();
        draw_entities();
        last_frame += FRAME_MS;
    }
    return 0;
}

int tots_remove_entity(uint8_t index) {
    if(index >= entity_array_index) return 1;
    int i = index;
    entity_array_index -= entity_array_index ? 1 : 0; 
    for(; i < entity_array_index; i++) {
        tots_entity_array[i] = tots_entity_array[i+1];
    }
    return 0;
}

tots_entity* add_entity(tots_entity_type type, uint16_t tag, int x, int y) {
    if(entity_array_index == MAX_ENTITIES || x < 1 || x > MAZE_X_LEN || y < 1 || y > MAZE_Y_LEN) {
        printk("add_entity(): Coordinates out of range or max entity count reached...\n");
        return NULL;
    }

    j_component* comp;
    switch(type) {
        case TOTS_PLAYER:
            comp = create_component_t(PLAYER_TAG,"entity",J_DECAL,22 + 28*(x-1),20+ 28*(y-1),Guy_0,&player_decal);
            break;
        default:
            printk("add_entity(): No valid type given...\n");
            return NULL;
            break;
    }

    tots_entity entity = {
        .type       = type,
        .anim       = false,
        .dirty      = 1,
        .move_x     = x,
        .move_y     = y,
        .prev_x     = x,
        .prev_y     = y,
        .x          = x,
        .y          = y,
        .ticks_per_move = 2,
        .tick_counter = 0,
        .tag = tag,
        .sprite = comp
    };
    tots_entity_array[entity_array_index++] = entity;
    return &tots_entity_array[entity_array_index - 1];
}

void draw_entities() {
    for(int i = 0; i < entity_array_index; i++) {
        tots_entity *cur_entity = &tots_entity_array[i];
        if(cur_entity->x < 1 || cur_entity->x > MAZE_X_LEN || cur_entity->y < 1 || cur_entity->y > MAZE_Y_LEN) continue;
        j_component *prev_decal = cur_entity->type == TOTS_PLAYER ? white_square : black_square;

        if(cur_entity->dirty) {
            if(cur_entity->sprite != NULL) {
                draw_component(cur_entity->sprite);
                if((cur_entity->x != cur_entity->prev_x) || (cur_entity->y != cur_entity->prev_y)) {
                    prev_decal->x = 22 + (cur_entity->prev_x-1) * GRID_SIZE;
                    prev_decal->y = 20 + (cur_entity->prev_y-1) * GRID_SIZE;
                    draw_component(prev_decal);
                }
            }
            cur_entity->dirty = 0;
        }
    }
}

int remove_entities(uint8_t tag) {
    for(int i = entity_array_index-1; i >= 0; i--) {
        if(tots_entity_array[i].tag == tag) tots_remove_entity(i);
    }
    return 0;
}

int move_entity(tots_entity* entity, int x, int y) {
    if(x == entity->x && y == entity->y) return 0; // no-op move, don't mark dirty

    entity->prev_x = entity->x;
    entity->prev_y = entity->y;

    entity->x = x;
    entity->y = y;

    if(entity->sprite == NULL) return 1;
    entity->sprite->x = 22 + (x-1) * GRID_SIZE;
    entity->sprite->y = 20 + (y-1) * GRID_SIZE;

    entity->dirty = 1;
    return 0;
}

int find_next_cell_move(swipe_dir dir, uint8_t* level_dat, int xo, int yo, int *xs, int *ys) {
    if(!dir) {
        *xs = xo;
        *ys = yo;
        return 1;
    }
    int x = xo, y = yo;
    int dx = 0, dy = 0;
    switch(dir) {
        case J_SWIPE_LEFT: dx = -1; break;
        case J_SWIPE_RIGHT: dx = 1; break;
        case J_SWIPE_UP: dy = -1; break;
        case J_SWIPE_DOWN: dy = 1; break;
    }

    int nx = x, ny = y;
    while(1) {
        nx += dx; ny += dy;
        if(nx > MAZE_X_LEN || nx < 1 || ny > MAZE_Y_LEN || ny < 1) break;
        if(level_dat[nx-1 + (ny-1)*MAZE_X_LEN]) break;
        x = nx; y = ny;
    }

    *xs = x;
    *ys = y;

    return 0;
}

int update_game() {

    swipe_dir SWIPE = get_swipe_touch_async();
    static uint8_t* player_dir_tex = Guy_0;
    static uint8_t* player_dir_tex_dash = Guy_Dash_0;
    switch(SWIPE) {
        case J_SWIPE_RIGHT:
            player_dir_tex = Guy_0;
            player_dir_tex_dash = Guy_Dash_0;
            break;
        case J_SWIPE_LEFT:
            player_dir_tex = Guy_180;
            player_dir_tex_dash = Guy_Dash_180;
            break;
        case J_SWIPE_UP:
            player_dir_tex = Guy_270;
            player_dir_tex_dash = Guy_Dash_270;
            break;
        case J_SWIPE_DOWN:
            player_dir_tex = Guy_90;
            player_dir_tex_dash = Guy_Dash_90;
            break;
    }
    // print_direction(SWIPE);
    for(int i = 0; i < entity_array_index; i++) {
        tots_entity *cur_entity = &tots_entity_array[i];
        cur_entity->tick_counter++;
        switch(cur_entity->type) {
            case TOTS_PLAYER:
                // Change direction sprite for main player. Do not change while dashing
                if(cur_entity->sprite == NULL) continue;
                if(cur_entity->move_x == cur_entity->x && cur_entity->move_y == cur_entity->y) {
                    cur_entity->sprite->dat = SWIPE ? player_dir_tex_dash : player_dir_tex;
                } 

                if(cur_entity->move_x == cur_entity->x && cur_entity->move_y == cur_entity->y) {
                    int goto_x, goto_y;
                    find_next_cell_move(SWIPE,cur_level,cur_entity->x,cur_entity->y,&goto_x,&goto_y);
                    cur_entity->move_x = goto_x; cur_entity->move_y = goto_y;
                }

                if(cur_entity->tick_counter % cur_entity->ticks_per_move) continue;
                int dx, dy;
                dx = dy = 0;
                if(cur_entity->x != cur_entity->move_x)
                    dx = cur_entity->move_x > cur_entity->x ? 1 : -1;
                else if(cur_entity->y != cur_entity->move_y)
                    dy = cur_entity->move_y > cur_entity->y ? 1 : -1;

                if(dx || dy)
                    move_entity(cur_entity,cur_entity->x + dx, cur_entity->y + dy);
                break;
        }
    }
    return 0;
}