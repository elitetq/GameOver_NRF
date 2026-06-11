#ifndef TOTS_LEVELS_H
#define TOTS_LEVELS_H

// Maze parameters
#define MAZE_X_LEN 7
#define MAZE_Y_LEN 10
#define MAZE_X_OFFSET 22
#define MAZE_Y_OFFSET 20

#include <inttypes.h>

int draw_level(uint8_t* level_dat);
extern const uint8_t* tots_level_textures[];

/*********************************************
                  Level Maps
 *********************************************/

extern const uint8_t tots_level_1[MAZE_X_LEN*MAZE_Y_LEN];


#endif