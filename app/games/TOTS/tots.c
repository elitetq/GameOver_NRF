#include <tots.h>
#include <tots_gl.h>
#include <tots_resource.h>
#include <stdbool.h>
#include <string.h>


//14 x 20

void tots_init(void) {
    init_graphics();
}

int draw_guy(int x, int y) {
    return draw_sprite(x,y,char_sprite);
}

#define LEVEL_W    (MAZE_X_LEN * GRID_SIZE)   // 196  full image width
#define LEVEL_H    (MAZE_Y_LEN * GRID_SIZE)   // 280  full image height
#define LEVEL_BODY ((LEVEL_W * LEVEL_H) / 8)  // packed 1bpp, no header

int draw_level(uint8_t* level_dat) {
    static uint8_t level_dat_comb[LEVEL_BODY + 4]; // in BSS, no heap needed

    // 4-byte decal header: [height_hi, height_lo, length_hi, length_lo]
    level_dat_comb[0] = (LEVEL_H >> 8) & 0xFF;
    level_dat_comb[1] = LEVEL_H & 0xFF;
    level_dat_comb[2] = (LEVEL_W >> 8) & 0xFF;
    level_dat_comb[3] = LEVEL_W & 0xFF;

    memset(level_dat_comb + 4, 0x00, LEVEL_BODY);

    for(size_t cell = 0; cell < MAZE_X_LEN * MAZE_Y_LEN; cell++) {
        const uint8_t* tex = (level_dat[cell] ? brick_texture : no_texture) + 4; // skip header
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

    j_decal_data level_decal_dat = {.animation_dat=NULL,.bg_col=BLACK,.col=DARK_RED};
    j_component* level_comp = create_component("level_comp",J_DECAL,22,20,(void*)level_dat_comb,&level_decal_dat);
    draw_component(level_comp);
    free_component(level_comp);   // frees the j_component struct (heap); buffer is static

    return 0;
}



