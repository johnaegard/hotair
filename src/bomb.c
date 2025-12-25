#include <cx16.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bomb.h"
#include "burning-petscii.h"

unsigned char bomb_index_get(unsigned char row, unsigned char col) {
  BANK_NUM = BOMB_BANK;
  return bomb_index[row][col];
}
void bomb_index_set(unsigned char row, unsigned char col, unsigned char value) {
  BANK_NUM = BOMB_BANK;
  bomb_index[row][col] = value;
}
void bombs_setup(void) {
  unsigned char b, bomb_x, bomb_y;
  for (bomb_y = 0; bomb_y < 64; bomb_y++) {
    for (bomb_x = 0; bomb_x < 64; bomb_x++) {
      bomb_index_set(bomb_y, bomb_x, 255);
    }
  }
  VERA.address_hi = 0 | VERA_INC_1;
  for (b = 0; b < NUM_BOMBS; b++) {
    do {
      bomb_x = rand() & 0x3F;  // 0-63
      bomb_y = rand() & 0x3F;  // 0-63
    } while (fire_index_get(bomb_y, bomb_x) < NO_FIRE_MAGIC_VALUE || water_index_get(bomb_y, bomb_x) > 0);

    bomb_index_set(bomb_y, bomb_x, b);
    bomb_pool[b].x = bomb_x;
    bomb_pool[b].y = bomb_y;
    bomb_pool[b].frame = 0;
    bomb_pool[b].exploding = false;
    bomb_pool[b].unexploded = true;

    VERA.address = vera_tilemap_addr_offsets[bomb_y][bomb_x];
    VERA.data0 = bomb_chars[0];
    VERA.data0 = bomb_colors[0];
  }
}
void bombs_blink(void) {
  unsigned char b = (game_frame % NUM_BOMBS);
  VERA.address_hi = 0 | VERA_INC_1;

  if (bomb_pool[b].unexploded) {
    VERA.address = vera_tilemap_addr_offsets[bomb_pool[b].y][bomb_pool[b].x];
    VERA.data0 = bomb_chars[bomb_pool[b].frame % NUM_UXBOMB_FRAMES];
    if (fire_index_get(bomb_pool[b].y, bomb_pool[b].x) < NO_FIRE_MAGIC_VALUE) {
    }
    else {
      VERA.data0 = bomb_colors[bomb_pool[b].frame % NUM_UXBOMB_FRAMES];
    }
    bomb_pool[b].frame++;
  }
}
void detonate_bomb(unsigned char b) {
  if (b < NUM_BOMBS && bomb_pool[b].unexploded) {
    bomb_pool[b].exploding = true;
    bomb_pool[b].unexploded = false;
    bomb_pool[b].frame = 1;
  }
}
void bombs_explode() {
  unsigned char prev_frame, b, tb;
  signed char dy, dx;
  signed char exp_y, exp_x;

  VERA.address_hi = 0 | VERA_INC_1;

  for (b = 0; b < NUM_BOMBS; b++) {

    if (!bomb_pool[b].exploding) {
      continue;
    }
    if (bomb_pool[b].frame == EXPLOSION_FRAMES) {
      bomb_pool[b].exploding = false;
    }

    for (dy = -4; dy <= 4; dy++) {
      exp_y = bomb_pool[b].y + dy;
      if (exp_y < 0 || exp_y >= MAP_WIDTH_TILES) {
        continue;
      }
      for (dx = -4; dx <= 4; dx++) {
        exp_x = bomb_pool[b].x + dx;
        if (exp_x < 0 || exp_x >= MAP_WIDTH_TILES) {
          continue;
        }
        prev_frame = (bomb_pool[b].frame - 1);

        // if the bomb is still exploding, paint leading edge of explosion
        if (bomb_pool[b].exploding && bomb_explosion_animation[bomb_pool[b].frame][dy + 4][dx + 4]) {
          VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x] +1;
          VERA.data0 = 0x01;
        }

        // trailing edge of explosion
        if (bomb_explosion_animation[prev_frame][dy + 4][dx + 4]) {

          // trigger other bombs
          tb = bomb_index_get(exp_y, exp_x);
          if (tb < NUM_BOMBS && bomb_pool[tb].unexploded) {
            if (rand() < EXPLOSION_DETONATE_CHANCE) {
              detonate_bomb(tb);
              continue;
            }
          }

          // just repaint water tiles
          if (water_index_get(exp_y, exp_x) > 0) {
            VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x];
            VERA.data0 = 0x60;
            continue;
          }

          // sometimes flatten land tiles
          if (water_index_get(exp_y, exp_x) == 0) {
            if (rand() < EXPLOSION_FLATTEN_CHANCE) {
              VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x];
              VERA.data0 = 0x66;
            }
          }

          // sometimes ignite non-burning land cells
          if (fire_index_get(exp_y, exp_x) == NO_FIRE_MAGIC_VALUE) {
            if (rand() < EXPLOSION_IGNITE_CHANCE) {
              fire_index_set(exp_y, exp_x, FIRE_DURATION);
              VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x] + 1;
              VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];
              continue;
            }
          }

          // sometimes burn out burning tiles
          else if (fire_index_get(exp_y, exp_x) < NO_FIRE_MAGIC_VALUE) {
            if (rand() < EXPLOSION_BURNOUT_CHANCE) {
              burn_out_tile(exp_y, exp_x);
              continue;
            }
          }
          VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x] + 1;
          VERA.data0 = 0x90;
        }
      }
    }
    bomb_pool[b].frame++;
  }
}
