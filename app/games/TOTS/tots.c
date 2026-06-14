#include <tots.h>
#include <tots_gl.h>
#include <tots_resource.h>
#include <stdbool.h>
#include <string.h>
#include <j_controls.h>

uint32_t last_frame, now_frame, fps;

#if TOTS_DEVELOPER_MODE 
  #define TOTS_LOG(fmt, ...)  printk("[TOTS] " fmt, ##__VA_ARGS__)
#else
  #define TOTS_LOG(fmt, ...)  ((void)0)
#endif

#define MAX_ENTITIES 25

static uint32_t game_tick = 0;
static int entity_array_index = 0;
static tots_entity tots_entity_array[MAX_ENTITIES] = {0};
static tots_entity *player_entity;
static bool next_level_flag = 0;
static bool prev_next_level_flag = 0;
uint8_t tots_point_field [MAZE_X_LEN][MAZE_Y_LEN];
uint16_t tots_points;

static uint8_t* cur_level;

#define TRAIL_TICK_LIFETIME 15
#define MAX_TRAIL_MEMORY 32
typedef struct {
    int x, y;
    uint32_t tick_created;
} trail_data;

static trail_data trail_field[MAX_TRAIL_MEMORY];
static int trail_index = 0;
static bool trail_end_pulse = 0;


#define FRAME_PER_SECOND 30
#define FRAME_MS 1000/FRAME_PER_SECOND



//14 x 20

tots_entity* add_entity(tots_entity_type type, swipe_dir default_facing, uint16_t tag, int x, int y);
const uint8_t* get_dir_sprite(const uint8_t* dir_tex[4], swipe_dir dir);
int generate_entities_from_level(const uint8_t* level_dat);
int tots_remove_entity(uint8_t index, bool free_sprite);

int check_death_condition();

const uint8_t* get_next_level() {
    if(!level_list_size) return NULL;
    if(cur_level == NULL) return level_list[0];

    if(level_list[game_tick%level_list_size] == cur_level) 
        return level_list[(game_tick+1)%level_list_size];
    return level_list[game_tick%level_list_size];
}

int tots_is_block_valid(int x, int y) {
    if(x < 1 || x > MAZE_X_LEN || y < 1 || y > MAZE_Y_LEN) return 0;
    return cur_level[(x-1) + MAZE_X_LEN*(y-1)] != 1;
}

int init_level(const uint8_t* level_dat) {
    last_frame = k_uptime_get_32();
    now_frame = k_uptime_get_32();
    cur_level = level_dat;
    draw_level(cur_level,tots_point_field);
    generate_entities_from_level(cur_level);
    tots_entity *plr = add_entity(TOTS_PLAYER,J_SWIPE_UP,PLAYER_TAG,MAZE_X_LEN/2+1,MAZE_Y_LEN+1);
    // add_entity(TOTS_PROJECTILE,J_SWIPE_UP,ENEMY_TAG,6,5);

    if(plr != NULL) {
        player_entity = plr;
        plr->move_x = MAZE_X_LEN/2+1;
        plr->move_y = MAZE_Y_LEN;
        return 0;
    }
    return 1;
}

int tots_init() {
    tots_points = 0;
    init_graphics();
    draw_borders(true);
    return init_level(get_next_level());
}

int draw_death_screen() {
    j_animation_data ds_anim = {.bg_col = BLACK, .increment_speed = 20, .percentage = 0, .type = FADE_IN};
    j_decal_data ds_decal = {.animation_dat = &ds_anim, .bg_col = BLACK, .col = RED};
    j_component* death_screen_comp = create_component_t(GUI_TAG,"death_screen",J_DECAL,MAZE_X_OFFSET,MAZE_Y_OFFSET,death_screen,&ds_decal);
    draw_component(death_screen_comp);
    free_component(death_screen_comp);
    return 0;
}

int draw_guy(int x, int y) {
    return draw_sprite(x,y,char_sprite);
}

int draw_entity(tots_entity *occupant) {
    if(occupant == NULL || occupant->sprite == NULL) return 1;

    bool has_trail = false;
    for(int i = 0; i < trail_index; i++) {
        if(trail_field[i].x == occupant->x && trail_field[i].y == occupant->y) { has_trail = true; break; }
    }

    if(has_trail) {
        j_decal_data *dd = (j_decal_data*)occupant->sprite->dat2;
        j_color saved_bg = dd->bg_col;
        dd->bg_col = PLAYER_COLOR;
        draw_component(occupant->sprite);
        dd->bg_col = saved_bg; // shared global decal — must restore for other entities
    } else {
        draw_component(occupant->sprite);
    }
    return 0;
}

int redraw_cell(int x, int y) {
    int px = MAZE_X_OFFSET + GRID_SIZE * (x-1);
    int py = MAZE_Y_OFFSET + GRID_SIZE * (y-1);

    // Is a live trail sitting on this cell?
    bool has_trail = false;
    for(int i = 0; i < trail_index; i++) {
        if(trail_field[i].x == x && trail_field[i].y == y) { has_trail = true; break; }
    }

    // Is an entity sitting on this cell? Lowest index wins (drawn on top, matching draw_entities).
    tots_entity *occupant = NULL;
    for(int i = 0; i < entity_array_index; i++) {
        if(tots_entity_array[i].x == x && tots_entity_array[i].y == y) { occupant = &tots_entity_array[i]; break; }
    }

    // Entity present: redraw it. If a trail is also here, fill its decal bg with the trail
    // color so the sprite reads as sitting on top of the trail.
    if(occupant != NULL && occupant->sprite != NULL) {
        occupant->sprite->x = px;
        occupant->sprite->y = py;
        if(has_trail) {
            j_decal_data *dd = (j_decal_data*)occupant->sprite->dat2;
            j_color saved_bg = dd->bg_col;
            dd->bg_col = PLAYER_COLOR;
            draw_component(occupant->sprite);
            dd->bg_col = saved_bg; // shared global decal — must restore for other entities
        } else {
            draw_component(occupant->sprite);
        }
        return 0;
    }

    // No entity: trail if present, else bare cell.
    j_component *empty_square = tots_point_field[x-1][y-1] ? point_square : black_square;
    j_component *fill = has_trail ? player_square : empty_square;
    fill->x = px;
    fill->y = py;
    draw_component(fill);
    return 0;
}

int remove_all_entities() {
    for(int i = entity_array_index-1; i >= 0; i--) {
        tots_remove_entity(i,true);
    }
}

int game_loop() {
    now_frame = k_uptime_get_32();
    if(player_entity == NULL) {
        TOTS_LOG("game_loop(): No valid player_entity pointer...");
        return 1;
    }

    if((now_frame - last_frame) >= FRAME_MS) {
        if(!(game_tick % FRAME_PER_SECOND)) {
            if(player_entity->data.dead_flag) TOTS_LOG("PLAYER DEAD...");
            else {
                TOTS_LOG("FPS: %d\n", fps);
                TOTS_LOG("Points: %d\n\n", tots_points);
                fps = (now_frame - last_frame) ? 1000/(now_frame-last_frame) : fps;
                tots_draw_points(tots_points);
            }
        }
        if(next_level_flag) { // Death logic
            trail_index = 0; // Clear trail
            int res = 1;
            bool dead_flag = player_entity->data.dead_flag;
            if(dead_flag) {
                draw_borders(true);
                tots_points = 0;
                res = draw_death_screen();
            } else { // Level completed
                prev_next_level_flag = 1;
                player_entity->move_x = MAZE_X_LEN/2 + 1;
                player_entity->move_y = 0;
                if(player_entity->prev_x != MAZE_X_LEN/2 + 1 || player_entity->prev_y != 0) {
                    update_game();
                    draw_entities();
                    return 0;
                }
            }
            black_square->x = MAZE_X_OFFSET + (MAZE_X_LEN/2)*GRID_SIZE;
            black_square->y = MAZE_Y_OFFSET - GRID_SIZE;
            k_msleep(200);
            remove_all_entities();
            draw_component(black_square);
            init_level(get_next_level());
            prev_next_level_flag = 0;
            next_level_flag = 0;
        } else {
            update_game();
            draw_entities();
        }
        last_frame += FRAME_MS;
    }
    return 0;
}

int tots_remove_entity(uint8_t index, bool free_sprite) {
    if(index >= entity_array_index || entity_array_index == 0) return 1;
    if(tots_entity_array[index].type == TOTS_PLAYER) player_entity = NULL;
    if(free_sprite) free_component(tots_entity_array[index].sprite); // sprite was never on the draw buffer, so freeing is enough
    tots_entity_array[index] = tots_entity_array[--entity_array_index];
    tots_entity_array[index].index = index;
    if(player_entity == &tots_entity_array[entity_array_index]) player_entity = &tots_entity_array[index]; // player got swapped into the freed slot
    return 0;
}

int generate_entities_from_level(const uint8_t* level_dat) {
    for(int i = 0; i < MAZE_X_LEN*MAZE_Y_LEN; i++) {
        if(level_dat[i] < 2) continue;
        int x = i%MAZE_X_LEN + 1;
        int y = i/MAZE_X_LEN + 1;
        switch(level_dat[i]) {
            case 2:
                add_entity(TOTS_DISPENSER,J_SWIPE_RIGHT,ENEMY_TAG,x,y);
                break;
            case 3:
                add_entity(TOTS_DISPENSER,J_SWIPE_DOWN,ENEMY_TAG,x,y);
                break;
            case 4:
                add_entity(TOTS_DISPENSER,J_SWIPE_LEFT,ENEMY_TAG,x,y);
                break;
            case 5:
                add_entity(TOTS_DISPENSER,J_SWIPE_UP,ENEMY_TAG,x,y);
                break;
            case 'b':
                add_entity(TOTS_BAT,J_SWIPE_RIGHT,ENEMY_TAG,x,y);
                break;
            case 'd':
                add_entity(TOTS_BAT,J_SWIPE_LEFT,ENEMY_TAG,x,y);
                break;
            case 'n':
                add_entity(TOTS_BAT,J_SWIPE_UP,ENEMY_TAG,x,y);
                break;
            case 'u':
                add_entity(TOTS_BAT,J_SWIPE_DOWN,ENEMY_TAG,x,y);
                break;
        }
    }
    return 0;
}

tots_entity* add_entity(tots_entity_type type, swipe_dir default_facing, uint16_t tag, int x, int y) {
    if(entity_array_index == MAX_ENTITIES || x < 1 || x > MAZE_X_LEN || y < 1 || y > MAZE_Y_LEN) {
        if(x != MAZE_X_LEN/2 + 1 || y != MAZE_Y_LEN+1) { // Check begin condition
            TOTS_LOG("add_entity(): Coordinates out of range or max entity count reached...\n");
            return NULL;
        }
    }

    entity_data data;
    uint32_t temp_tpm; // ticks per move

    j_component* comp;
    switch(type) {
        //                              ENTITY: PLAYER
        case TOTS_PLAYER: {
            temp_tpm = FRAME_PER_SECOND/10;
            entity_data temp = {
                .dead_flag = 0,
                .despawn_flag = 0,
                .despawn_ticks = temp_tpm,
                .dir_tex = {Guy_0,Guy_90,Guy_180,Guy_270},
                .secondary_dir_tex = {Guy_Dash_0, Guy_Dash_90, Guy_Dash_180, Guy_Dash_270},
                .facing = default_facing
            };
            data = temp;
            comp = create_component_t(PLAYER_TAG,"player",J_DECAL,MAZE_X_OFFSET + GRID_SIZE*(x-1),MAZE_Y_OFFSET+ GRID_SIZE*(y-1),(void*)get_dir_sprite(temp.dir_tex,default_facing),&player_decal);
            break;
        }
        //                              ENTITY: PROJECTILE
        case TOTS_PROJECTILE: {
            temp_tpm = FRAME_PER_SECOND/8;
            entity_data temp = {
                .dead_flag = 0,
                .despawn_flag = 0,
                .despawn_ticks = temp_tpm,
                .dir_tex = {Arrow_0,Arrow_90,Arrow_180,Arrow_270},
                .secondary_dir_tex = {NULL,NULL,NULL,NULL},
                .facing = default_facing
            };
            data = temp;
            comp = create_component_t(ENEMY_TAG,"projectile",J_DECAL,MAZE_X_OFFSET + GRID_SIZE*(x-1),MAZE_Y_OFFSET + GRID_SIZE*(y-1),(void*)get_dir_sprite(temp.dir_tex,default_facing),&enemy_decal);
            break;
        }
        //                              ENTITY: DISPENSER
        case TOTS_DISPENSER: {
            temp_tpm = (FRAME_PER_SECOND*5)/2; // tpm is translated to firing speed for a dispenser
            entity_data temp = {
                .dead_flag = 0,
                .despawn_flag = 0,
                .despawn_ticks = temp_tpm,
                .dir_tex = {Dispenser_0,Dispenser_90,Dispenser_180,Dispenser_270},
                .secondary_dir_tex = {NULL,NULL,NULL,NULL},
                .facing = default_facing
            };
            data = temp;
            comp = create_component_t(ENEMY_TAG,"dispenser",J_DECAL,MAZE_X_OFFSET + GRID_SIZE*(x-1),MAZE_Y_OFFSET + GRID_SIZE*(y-1),(void*)get_dir_sprite(temp.dir_tex,default_facing),&enemy_decal);
            break;
        }
        //                              ENTITY: BAT
        case TOTS_BAT: {
            temp_tpm = (FRAME_PER_SECOND)/3;
            int px2, py2;
            find_next_cell_move(default_facing,cur_level,x,y,&px2,&py2);
            entity_data temp = {
                .dead_flag = 0,
                .despawn_flag = 0,
                .despawn_ticks = temp_tpm,
                .dir_tex = {Bat_Flap1,Bat_Flap2,Bat_Flap1,Bat_Flap2},
                .secondary_dir_tex = {NULL,NULL,NULL,NULL},
                .facing = J_SWIPE_RIGHT,
                .px1 = x,
                .py1 = y,
                .px2 = px2,
                .py2 = py2

            };
            data = temp;
            comp = create_component_t(ENEMY_TAG,"bat",J_DECAL,MAZE_X_OFFSET + GRID_SIZE*(x-1),MAZE_Y_OFFSET + GRID_SIZE*(y-1),(void*)get_dir_sprite(temp.dir_tex,J_SWIPE_RIGHT),&enemy_decal);
            break;
        }
        default:
            TOTS_LOG("add_entity(): No valid type given...\n");
            return NULL;
            break;
    }

    tots_entity entity = {
        .index      = entity_array_index,
        .type       = type,
        .dirty      = 1,
        .move_x     = x,
        .move_y     = y,
        .prev_x     = x,
        .prev_y     = y,
        .internal_ticks = 0,
        .ticks_snapshot = 0,
        .x          = x,
        .y          = y,
        .ticks_per_move = temp_tpm,
        .tag        = tag,
        .sprite     = comp,
        .data       = data
    };
    tots_entity_array[entity_array_index] = entity;
    return &tots_entity_array[entity_array_index++];
}

void draw_entities() {

    // Drawing player trail
    for(int i = trail_index-1; i >= 0; i--) {
        if(game_tick - trail_field[i].tick_created > TRAIL_TICK_LIFETIME) {
            int tx = trail_field[i].x, ty = trail_field[i].y;
            trail_field[i] = trail_field[--trail_index]; // Remove first so redraw_cell doesn't restore this trail
            if(!trail_index) trail_end_pulse = 1; // update player sprite with no trail if trail_index is 0

            // Don't erase under the player (redraw_cell can't restore it without entity sifting)
            if(player_entity == NULL || player_entity->x != tx || player_entity->y != ty)
                redraw_cell(tx, ty);
        }
    }

    // Drawing entities
    for(int i = entity_array_index-1; i >= 0; i--) {
        tots_entity *cur_entity = &tots_entity_array[i];
        if(cur_entity->x < 1 || cur_entity->x > MAZE_X_LEN || (cur_entity->y < 1 && cur_entity->x != MAZE_X_LEN/2+1) || cur_entity->y > MAZE_Y_LEN) continue;
        bool is_player = cur_entity->type == TOTS_PLAYER;

        if(cur_entity->data.despawn_flag) { // Despawn logic
            if(cur_entity->data.despawn_ticks--) continue;
            int px = cur_entity->prev_x, py = cur_entity->prev_y;
            tots_remove_entity(cur_entity->index,true); // remove first so redraw_cell doesn't find this entity as the occupant
            redraw_cell(px, py);
            continue;
        }

        if(cur_entity->dirty) {
            if(cur_entity->sprite != NULL) {
                draw_entity(cur_entity);
                // draw_component(cur_entity->sprite);

                if((cur_entity->x != cur_entity->prev_x) || (cur_entity->y != cur_entity->prev_y)) {
                    player_square->x = MAZE_X_OFFSET + (cur_entity->prev_x-1) * GRID_SIZE;
                    player_square->y = MAZE_Y_OFFSET + (cur_entity->prev_y-1) * GRID_SIZE;
                    if(is_player) {
                        if(((player_entity->prev_x != player_entity->x) || (player_entity->prev_y != player_entity->y)) && trail_index < MAX_TRAIL_MEMORY) {
                            trail_field[trail_index++] = (trail_data){
                                .tick_created = game_tick,
                                .x = cur_entity->prev_x,
                                .y = cur_entity->prev_y
                            };
                            draw_component(player_square);
                        }

                    } else {
                        redraw_cell(cur_entity->prev_x, cur_entity->prev_y);
                    }
                    // Trail effect generator
                }
            }
            cur_entity->dirty = 0;
        }
    }
}

int remove_entities(uint8_t tag, bool free_sprite) {
    for(int i = entity_array_index-1; i >= 0; i--) {
        if(tots_entity_array[i].tag == tag) tots_remove_entity(i,free_sprite);
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
    entity->sprite->x = MAZE_X_OFFSET + (x-1) * GRID_SIZE;
    entity->sprite->y = MAZE_Y_OFFSET + (y-1) * GRID_SIZE;

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
    char exclude_list[] = {6, 'b', 'd', 'u', 'n'}; // You can go through all of these 'blocks'
    while(1) {
        nx += dx; ny += dy;
        if(nx > MAZE_X_LEN || nx < 1 || ny > MAZE_Y_LEN || ny < 1) break;
        uint8_t item = level_dat[nx-1 + (ny-1)*MAZE_X_LEN];
        if(item) {
            bool break_while = true;
            for(int i = 0; i < sizeof(exclude_list); i++) {
                if(item == exclude_list[i]) {
                    break_while = false;
                    break;
                } 
            }
            if(break_while) break;
        }
        x = nx; y = ny;
    }

    *xs = x;
    *ys = y;

    return 0;
}

// Picks the directional sprite that matches a swipe direction.
// dir_tex is ordered {0 deg, 90 deg, 180 deg, 270 deg}.
// Returns NULL when the swipe has no matching direction.
const uint8_t* get_dir_sprite(const uint8_t* dir_tex[4], swipe_dir dir) {
    switch(dir) {
        case J_SWIPE_RIGHT: return dir_tex[0]; // 0 deg
        case J_SWIPE_DOWN:  return dir_tex[1]; // 90 deg
        case J_SWIPE_LEFT:  return dir_tex[2]; // 180 deg
        case J_SWIPE_UP:    return dir_tex[3]; // 270 deg
        default:            return NULL;
    }
}

int update_game() {
    game_tick++;
    swipe_dir SWIPE = get_swipe_touch_async();
    if(player_entity != NULL && SWIPE == J_SWIPE_UP && player_entity->x == MAZE_X_LEN/2+1 && player_entity->y == 1) {
        next_level_flag = 1; // Next level, exiting from the top
        player_entity->data.facing = J_SWIPE_UP;
        return 0;
    }

    /**************************************************************
                            ENTITY LOGIC
     **************************************************************/
    for(int i = 0; i < entity_array_index; i++) {
        tots_entity *cur_entity = &tots_entity_array[i];

        // Helpful global variables
        bool is_stationary = (cur_entity->move_x == cur_entity->x) && (cur_entity->move_y == cur_entity->y);
        entity_data *entity_dat = &cur_entity->data;
        swipe_dir entity_dir = entity_dat->facing;
        bool is_dead = entity_dat->dead_flag;
        bool is_player = cur_entity->type == TOTS_PLAYER;

        if(cur_entity == NULL || cur_entity->sprite == NULL) continue;
        if(is_stationary) { // Stops hanging prev_y/prev_x 
            cur_entity->prev_x = cur_entity->x;
            cur_entity->prev_y = cur_entity->y;
        }
        cur_entity->internal_ticks++;

        bool update_flag = !(cur_entity->internal_ticks % cur_entity->ticks_per_move);


        switch(cur_entity->type) {
            case TOTS_PLAYER: {

                if(trail_end_pulse) { // Handle edge case to change player sprite back to normal
                    player_entity->dirty = 1;
                    trail_end_pulse = 0;
                }
                if(is_stationary) { // Swipe cell move logic
                    if(SWIPE) entity_dat->facing = SWIPE;
                    int goto_x, goto_y;
                    find_next_cell_move(SWIPE,cur_level,cur_entity->x,cur_entity->y,&goto_x,&goto_y);
                    cur_entity->move_x = goto_x; cur_entity->move_y = goto_y;
                }

                // Trail sprite change logic (If trail exists or you are moving)
                is_stationary = (cur_entity->move_x == cur_entity->x) && (cur_entity->move_y == cur_entity->y);
                cur_entity->sprite->dat = (trail_index || !is_stationary) ? get_dir_sprite(entity_dat->secondary_dir_tex,entity_dat->facing) : get_dir_sprite(entity_dat->dir_tex,entity_dat->facing);


                break;
            }
            case TOTS_PROJECTILE: {
                if(is_stationary && !is_dead) { // When projectile spawns, set move position
                    cur_entity->data.dead_flag = 1; // Spawn flag for projectiles, determine direction on spawn
                    int goto_x, goto_y;
                    find_next_cell_move(entity_dir,cur_level,cur_entity->x,cur_entity->y,&goto_x,&goto_y);
                    cur_entity->move_x = goto_x; cur_entity->move_y = goto_y;
                } else if(is_stationary && is_dead) { // Despawn logic (hit a wall or end)
                    entity_dat->despawn_flag = 1;
                }

                break;
            }
            case TOTS_DISPENSER: {

                if(cur_entity->data.dead_flag && cur_entity->internal_ticks - cur_entity->ticks_snapshot > 10) { // Highlight for 0.33 seconds
                    cur_entity->data.dead_flag = 0;
                    cur_entity->sprite->dat2 = (void*)&enemy_decal;
                    cur_entity->dirty = 1;
                }
                if(!update_flag) continue;
                int dx, dy;
                switch(entity_dat->facing) { // Projectile spawner (arrow)
                    case J_SWIPE_RIGHT:
                        if(cur_entity->x >= MAZE_X_LEN) continue;
                        dx = 1; dy = 0;
                        break;
                    case J_SWIPE_LEFT:
                        if(cur_entity->x <= 1) continue;
                        dx = -1; dy = 0;
                        break;
                    case J_SWIPE_UP:
                        if(cur_entity->y <= 1) continue;
                        dx = 0; dy = -1;
                        break;
                    case J_SWIPE_DOWN:
                        if(cur_entity->y >= MAZE_Y_LEN) continue;
                        dx = 0; dy = 1;
                        break;
                    default:
                        dx = 0; dy = 0;
                        break;
                }

                add_entity(TOTS_PROJECTILE,entity_dat->facing,ENEMY_TAG,cur_entity->x + dx, cur_entity->y + dy);
                cur_entity->sprite->dat2 = (void*)&highlight_decal;
                cur_entity->data.dead_flag = 1;
                cur_entity->ticks_snapshot = cur_entity->internal_ticks;
                cur_entity->dirty = 1;
                break;
            }
            case TOTS_BAT: {
                if(is_stationary) { // Swipe cell move logic
                    if(SWIPE) entity_dat->facing = SWIPE;
                    int goto_x, goto_y;
                    find_next_cell_move(SWIPE,cur_level,cur_entity->x,cur_entity->y,&goto_x,&goto_y);
                    cur_entity->move_x = goto_x; cur_entity->move_y = goto_y;
                }
            }
        }

        /***************************************************
                            Move Logic
         ***************************************************/
        // Update is_stationary
        is_stationary = (cur_entity->move_x == cur_entity->x) && (cur_entity->move_y == cur_entity->y);
        if(!update_flag || is_stationary) continue;

        int dx, dy;
        dx = dy = 0;
        if(cur_entity->x != cur_entity->move_x)
            dx = cur_entity->move_x > cur_entity->x ? 1 : -1;
        else if(cur_entity->y != cur_entity->move_y)
            dy = cur_entity->move_y > cur_entity->y ? 1 : -1;

        if(dx || dy)
            move_entity(cur_entity,cur_entity->x + dx, cur_entity->y + dy);

        if(is_player) {
            // check_death_condition();
            if(cur_entity->x < 1 || cur_entity->x > MAZE_X_LEN || cur_entity->y < 1 || cur_entity->y > MAZE_Y_LEN) continue;
            if(tots_point_field[cur_entity->x-1][cur_entity->y-1]) {
                tots_point_field[cur_entity->x-1][cur_entity->y-1] = 0;
                tots_points++;
            }
        }
    }
    check_death_condition();
    return 0;
}

int check_death_condition() { // Will always enter only if player_entity is valid
    for(int i = entity_array_index-1; i >= 0; i--) {
        if(i == player_entity->index) continue;
        bool player_touched_entity = tots_entity_array[i].x == player_entity->x && tots_entity_array[i].y == player_entity->y;
        bool player_swiped_entity_x = tots_entity_array[i].prev_x == player_entity->x && tots_entity_array[i].x == player_entity->prev_x && player_entity->y == tots_entity_array[i].y;
        bool player_swiped_entity_y = tots_entity_array[i].prev_y == player_entity->y && tots_entity_array[i].y == player_entity->prev_y && player_entity->x == tots_entity_array[i].x;
        if(player_touched_entity || player_swiped_entity_x || player_swiped_entity_y) {
            player_entity->data.dead_flag = 1;
            next_level_flag = 1;
            return 0;
        }
    }
    return 0;
}