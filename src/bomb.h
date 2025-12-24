#ifndef BOMB_H
#define BOMB_H

#include <stdbool.h>

#define NUM_BOMBS 16
#define NUM_UXBOMB_FRAMES 4
#define BOMB_BURNING_DETONATION_CHANCE 16000
#define EXPLOSION_FRAMES 7
#define EXPLOSION_SIZE_TILES 9
#define EXPLOSION_IGNITE_CHANCE 3000
#define EXPLOSION_FLATTEN_CHANCE 16000
#define EXPLOSION_BURNOUT_CHANCE 16000
#define EXPLOSION_DETONATE_CHANCE 30000

// Function declarations
unsigned char bomb_index_get(unsigned char row, unsigned char col);
void bomb_index_set(unsigned char row, unsigned char col, unsigned char value);
void bombs_setup(void);
void bombs_blink(void);
void detonate_bomb(unsigned char b);
void bombs_explode(void);

#endif // BOMB_H
