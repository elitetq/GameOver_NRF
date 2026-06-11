#include <tots_levels.h>
#include <tots.h>
#include <tots_resource.h>

#define LEVEL_W    (MAZE_X_LEN * GRID_SIZE)   // 196  full image width
#define LEVEL_H    (MAZE_Y_LEN * GRID_SIZE)   // 280  full image height
#define LEVEL_BODY ((LEVEL_W * LEVEL_H) / 8)  // packed 1bpp, no header


const uint8_t* tots_level_textures[] = {no_texture,brick_texture,Dispenser_0,Dispenser_90,Dispenser_180,Dispenser_270};

#define DIC_SIZE 6
/***************************************************
                    Dictionary

    0 - Empty space
    1 - Brick wall
    2 - Dispenser (facing right)
    3 - Dispenser (facing down)
    4 - Dispenser (facing left)
    5 - Dispenser (facing up)
 ***************************************************/

const uint8_t tots_level_1[MAZE_X_LEN*MAZE_Y_LEN] = {
    0, 0, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 0, 1,
    0, 1, 0, 0, 0, 0, 1,
    0, 1, 0, 0, 1, 0, 1,
    0, 2, 0, 0, 0, 0, 0,
    0, 1, 0, 1, 0, 1, 0,
    0, 0, 0, 1, 0, 1, 0,
    0, 0, 0, 1, 0, 1, 0,
    0, 1, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 0,
};


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
        if(level_dat[local_cell] >= DIC_SIZE || level_dat[local_cell] < 0) continue;
        const uint8_t* tex = (tots_level_textures[level_dat[local_cell]]) + 4; // skip header
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
    j_component* level_comp = create_component("level_comp",J_DECAL,MAZE_X_OFFSET,MAZE_Y_OFFSET,(void*)level_dat_comb,&level_decal_dat);
    draw_component(level_comp);
    free_component(level_comp);   // frees the j_component struct (heap); buffer is static

    return 0;
}
