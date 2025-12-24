#ifndef BOMB_H
#define BOMB_H

#include <stdbool.h>

// Function declarations
unsigned char bomb_index_get(unsigned char row, unsigned char col);
void bomb_index_set(unsigned char row, unsigned char col, unsigned char value);
void bombs_setup(void);
void bombs_blink(void);
void detonate_bomb(unsigned char b);
void bombs_explode(void);

#endif // BOMB_H
