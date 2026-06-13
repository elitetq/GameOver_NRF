#ifndef TOTS_LEVELS_H
#define TOTS_LEVELS_H

// Maze parameters
#define MAZE_X_LEN 7
#define MAZE_Y_LEN 10
#define MAZE_X_OFFSET 22
#define MAZE_Y_OFFSET 20

#include <inttypes.h>

int draw_level(const uint8_t* level_dat, uint8_t point_field[MAZE_X_LEN][MAZE_Y_LEN]);
extern const uint8_t* tots_level_textures[];
extern const uint8_t* level_list[];
extern const uint8_t level_list_size;

/*********************************************
                  Level Maps
 *********************************************/

extern const uint8_t tots_level_1[MAZE_X_LEN*MAZE_Y_LEN];
extern const uint8_t tots_level_2[MAZE_X_LEN*MAZE_Y_LEN];
extern const uint8_t tots_level_3[MAZE_X_LEN*MAZE_Y_LEN];
extern const uint8_t tots_level_4[MAZE_X_LEN*MAZE_Y_LEN];
extern const uint8_t tots_level_5[MAZE_X_LEN*MAZE_Y_LEN];


#endif