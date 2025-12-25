#include "burning-petscii.h"

#include <cbm.h>
#include <cx16.h>
#include <joystick.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bomb.h"
#include "map.h"
#include "vera-util.h"
#include "wait.h"

unsigned char fire_colors[FIRE_COLOR_COMBINATIONS] = {0x22, 0x77, 0x88, 0XAA, 0x22, 0x77, 0x88, 0XAA, 0x77,
                                                      0x87, 0XA7, 0x71, 0x81, 0xA1, 0x77, 0x88, 0x77, 0x88};

char (*fire_index)[64] = (char (*)[64])FIRE_DATA_ADDRESS;
char (*water_index)[64] = (char (*)[64])WATER_DATA_ADDRESS;
char (*bomb_index)[64] = (char (*)[64])BOMB_DATA_ADDRESS;
char (*map_tiles_index)[64][2] = (char (*)[64][2])MAP_TILES_DATA_ADDRESS;

unsigned char bomb_colors[NUM_UXBOMB_FRAMES] = {0x00, 0x01, 0x00, 0x03};
unsigned char bomb_chars[NUM_UXBOMB_FRAMES] = {0x00, 0x57, 0x00, 0x5A};
unsigned char bomb_explosion_animation[EXPLOSION_FRAMES][EXPLOSION_SIZE_TILES][EXPLOSION_SIZE_TILES] = {
    {// FRAME 0
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 1, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {// FRAME 1
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 1, 0, 0, 0, 0},
     {0, 0, 0, 1, 0, 1, 0, 0, 0},
     {0, 0, 0, 0, 1, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {// FRAME 2
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 1, 0, 0, 0, 0},
     {0, 0, 0, 1, 0, 1, 0, 0, 0},
     {0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 0, 0, 1, 0, 1, 0, 0, 0},
     {0, 0, 0, 0, 1, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {// FRAME 3
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 1, 1, 1, 0, 0, 0},
     {0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 0, 0, 1, 1, 1, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {// FRAME 4
     {0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 1, 1, 1, 0, 0, 0},
     {0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 1, 0, 0, 0, 0, 0, 1, 0},
     {0, 1, 0, 0, 0, 0, 0, 1, 0},
     {0, 1, 0, 0, 0, 0, 0, 1, 0},
     {0, 0, 1, 0, 0, 0, 1, 0, 0},
     {0, 0, 0, 1, 1, 1, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {// FRAME 5
     {0, 0, 1, 1, 1, 1, 1, 0, 0},
     {0, 1, 0, 0, 0, 0, 0, 1, 0},
     {1, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 1},
     {1, 0, 0, 0, 0, 0, 0, 0, 1},
     {0, 1, 0, 0, 0, 0, 0, 1, 0},
     {0, 0, 1, 1, 1, 1, 1, 0, 0}}};

Person people_pool[MAX_PEOPLE];
unsigned int people_count = 0;

const signed char people_dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
const signed char people_dy[8] = {1, 1, 1, 0, 0, -1, -1, -1};

unsigned int pm_idx;
signed char pm_dir;
signed char pm_nx;
signed char pm_ny;

SpriteFrame sprite_frame_data = {0, 0};
SpriteFrame* sprite_frame = &sprite_frame_data;
Bomb bomb_pool[NUM_BOMBS];

unsigned long game_frame = 0;
unsigned char areg;
unsigned char rand_x, rand_y;
unsigned long addr;
unsigned int sample;
unsigned int vera_tilemap_addr_offsets[64][64];
const signed char fire_xpread[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
const signed char fire_ypread[8] = {1, 1, 1, 0, 0, -1, -1, -1};
signed char dieroll;
signed char spread_x, spread_y, keycode;
unsigned int die_roll;  /// ugh
signed char wind_direction = 12;
signed char needle_sprite_frame;

void random_setup(void) {
  // call entropy_get to seed the random number generator
  asm("jsr $FECF");
  asm("STA %v", areg);  // Added missing '&' for address reference
  srand(areg);
}
void mouse_setup(void) {
  asm("sec");
  asm("ldx #80");
  asm("ldy #60");
  asm("lda #1");
  asm("jsr $FF68");  // Call mouse_config Kernal Function
  wait();            // Wait a cycle for the mouse to fully activate
}
void vera_screen_setup(void) {
  VERA.display.video = SPRITES_ENABLED | LAYER1_ENABLED | LAYER0_ENABLED | VGA_ENABLED;

  VERA.display.hscale = DC_HSCALE_640;
  VERA.display.vscale = DC_VSCALE_480;

  VERA.layer0.mapbase = (MAP0_ADDR >> 9);

  VERA.layer0.config = LAYER_MAP_HEIGHT_64 | LAYER_MAP_WIDTH_64 | LAYER_T256C_OFF | LAYER_BITMAP_OFF | LAYER_BPP_1;

  VERA.layer0.tilebase = (TILESET_ADDR >> 9 & TILE_BASE_ADDR_MASK) | TILE_HEIGHT_8PX | TILE_WIDTH_8PX;

  VERA.layer1.config = LAYER_MAP_HEIGHT_64 | LAYER_MAP_WIDTH_128 | LAYER_T256C_OFF | LAYER_BITMAP_OFF | LAYER_BPP_1;

  VERA.layer1.tilebase = (TILESET_ADDR >> 9 & TILE_BASE_ADDR_MASK) | TILE_HEIGHT_8PX | TILE_WIDTH_8PX;

  VERA.layer1.mapbase = (MAP1_ADDR >> 9);
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
}
void vera_loads(void) {
  printf("loading\n");
  load_into_vera("overlay.bin", MAP1_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("needle.bin", NEEDLE_SPRITE_BITMAP_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("circle.bin", CIRCLE_SPRITE_BITMAP_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("face.bin", FACE_SPRITE_BITMAP_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("tiles.bin", TILESET_ADDR, SKIP_2_BYTE_HEADER);
}

// BANKED SHIT
unsigned char fire_index_get(unsigned char row, unsigned char col) {
  BANK_NUM = FIRE_AND_WATER_BANK;
  return fire_index[row][col];
}
void fire_index_set(unsigned char row, unsigned char col, char value) {
  BANK_NUM = FIRE_AND_WATER_BANK;
  fire_index[row][col] = value;
}
unsigned char water_index_get(unsigned char row, unsigned char col) {
  BANK_NUM = FIRE_AND_WATER_BANK;
  return water_index[row][col];
}
void water_index_set(unsigned char row, unsigned char col, char value) {
  BANK_NUM = FIRE_AND_WATER_BANK;
  water_index[row][col] = value;
}
unsigned char map_tiles_index_get(unsigned char row, unsigned char col, unsigned char layer) {
  BANK_NUM = MAP_TILES_BANK;
  return map_tiles_index[row][col][layer];
}
void map_tiles_index_set(unsigned char row, unsigned char col, unsigned char layer, char value) {
  BANK_NUM = MAP_TILES_BANK;
  map_tiles_index[row][col][layer] = value;
}

// SPRITES
void sprite24_frame(SpriteFrame* sf, unsigned long base_addr, unsigned int frame_size_bytes, unsigned char frame) {
  if (frame >= 19) {
    sf->flips = 0b01;
    sf->frame_addr = base_addr + (frame_size_bytes * (24 - frame));
  } else if (frame >= 13) {
    sf->flips = 0b11;
    sf->frame_addr = base_addr + (frame_size_bytes * (frame - 12));
  } else if (frame >= 7) {
    sf->flips = 0b10;
    sf->frame_addr = base_addr + (frame_size_bytes * (12 - frame));
  } else {
    sf->flips = 0b00;
    sf->frame_addr = base_addr + frame_size_bytes * frame;
  }
}

// FIRE
void fire_color_setup(void) {
  unsigned char fg_index, bg_index, lookup_index;
  lookup_index = 0;
  for (bg_index = 0; bg_index < FIRE_NUM_BG_COLORS; bg_index++) {
    for (fg_index = 0; fg_index < FIRE_NUM_FG_COLORS; fg_index++) {
      //      fire_colors[lookup_index] = (fire_bgcolors[bg_index] | fire_fgcolors[fg_index]);
      lookup_index++;
    }
  }
}

void vera_tilemap_addr_offsets_setup(void) {
  unsigned char r, c;
  for (r = 0; r < MAP_HEIGHT_TILES; r++) {
    for (c = 0; c < MAP_WIDTH_TILES; c++) {
      vera_tilemap_addr_offsets[r][c] = (2 * (r * MAP_WIDTH_TILES + c));
    }
  }
}
void fire_setup(void) {
  unsigned char c, r;
  unsigned long addr;
  unsigned char neighbor_count, pass;
  unsigned int tiles_processed = 0;

  // Initial random seeding - lower probability
  for (r = 0; r < 64; r++) {
    for (c = 0; c < 64; c++) {
      die_roll = rand();
      if (die_roll < FIRE_SEED_CHANCE) {
        fire_index_set(r, c, FIRE_DURATION);
        water_index_set(r, c, 0);
      } else if (die_roll < SOAKED_SEED_CHANCE) {
        water_index_set(r, c, SOAK_DURATION);
      } else {
        fire_index_set(r, c, NO_FIRE_MAGIC_VALUE);
        water_index_set(r, c, 0);
      }
    }
  }

  printf("\nseeding water:");

  for (pass = 0; pass < 3; pass++) {
    for (r = 1; r < 63; r++) {  // Skip edges to avoid boundary checks
      for (c = 1; c < 63; c++) {
        if (water_index_get(r, c) == 0) {
          neighbor_count = 0;

          // Count neighbors (8-connected)
          if (water_index_get(r - 1, c - 1) > 0)
            neighbor_count++;
          if (water_index_get(r - 1, c) > 0)
            neighbor_count++;
          if (water_index_get(r - 1, c + 1) > 0)
            neighbor_count++;
          if (water_index_get(r, c - 1) > 0)
            neighbor_count++;
          if (water_index_get(r, c + 1) > 0)
            neighbor_count++;
          if (water_index_get(r + 1, c - 1) > 0)
            neighbor_count++;
          if (water_index_get(r + 1, c) > 0)
            neighbor_count++;
          if (water_index_get(r + 1, c + 1) > 0)
            neighbor_count++;

          // Higher probability based on neighbor count
          if (neighbor_count > 0) {
            unsigned int threshold = neighbor_count * 4800;
            if (rand() < threshold) {
              water_index_set(r, c, SOAK_DURATION);
            }
          }
        }
        if (tiles_processed++ % 1000 == 0) {
          printf("%1c%1c%1c", 30, 0x63, 5);
        }
      }
    }
  }

  tiles_processed = 0;
  printf("\nseeding fire: ");

  for (pass = 0; pass < 3; pass++) {
    for (r = 1; r < 63; r++) {  // Skip edges to avoid boundary checks
      for (c = 1; c < 63; c++) {
        if (fire_index_get(r, c) == NO_FIRE_MAGIC_VALUE && water_index_get(r, c) == 0) {
          neighbor_count = 0;

          // Count neighbors (8-connected)
          if (fire_index_get(r - 1, c - 1) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r - 1, c) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r - 1, c + 1) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r, c - 1) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r, c + 1) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r + 1, c - 1) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r + 1, c) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;
          if (fire_index_get(r + 1, c + 1) < NO_FIRE_MAGIC_VALUE)
            neighbor_count++;

          // Higher probability based on neighbor count
          if (neighbor_count > 0) {
            unsigned int threshold = neighbor_count * 4500;
            if (rand() < threshold) {
              fire_index_set(r, c, FIRE_DURATION);
            }
          }
        }
        if (tiles_processed++ % 1000 == 0) {
          printf("%1c%1c%1c", 30, 0x63, 5);
        }
      }
    }
  }
}
void burn_out_tile(unsigned char y, unsigned char x) {
  fire_index_set(y, x, BURNT_OUT);
  if ((rand() < BURNT_OUT_CRUMBLE_TILE_CHANCE)) {
    VERA.address = vera_tilemap_addr_offsets[y][x];
    VERA.data0 = 0x66;
  } else {
    VERA.address = vera_tilemap_addr_offsets[y][x] + 1;
  }
  VERA.data0 = 0xB0;
}
void burn() {
  unsigned char bomb_id;

  VERA.address_hi = 0 | VERA_INC_1;

  for (sample = 0; sample < FIRE_SAMPLES_PER_FRAME; sample++) {
    rand_x = rand() & 0x3F;  // 0-63 (mask with 0011 1111)
    rand_y = rand() & 0x3F;  // 0-63 (mask with 0011 1111)

    //
    // YOU DO NOT BURN
    //
    if (fire_index_get(rand_y, rand_x) > FIRE_DURATION) {
      continue;
    }

    //
    // YOU BURN
    //
    fire_index_set(rand_y, rand_x, fire_index_get(rand_y, rand_x) - 1);

    //
    // YOU TRIGGER A BOMB
    //
    bomb_id = bomb_index_get(rand_y, rand_x);
    if (rand() < BOMB_BURNING_DETONATION_CHANCE) {
      detonate_bomb(bomb_id);
    }

    //
    // YOU BURN OUT
    //
    if (fire_index_get(rand_y, rand_x) == 0) {
      burn_out_tile(rand_y, rand_x);
      continue;
    }

    //
    // YOU TWINKLE
    //
    VERA.address = vera_tilemap_addr_offsets[rand_y][rand_x] + 1;
    VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];

    //
    // YOU SPREAD
    //
    if (rand() < FIRE_SPREAD_CHANCE) {
      dieroll = rand() % 7;
      spread_x = rand_x + fire_xpread[dieroll];
      spread_y = rand_y + fire_ypread[dieroll];
      if (spread_x >= 0 && spread_x < 64 && spread_y >= 0 && spread_y < 64) {
        if (water_index_get(spread_y, spread_x) > 0) {
          water_index_set(spread_y, spread_x, water_index_get(spread_y, spread_x) - 1);
        } else if (fire_index_get(spread_y, spread_x) == NO_FIRE_MAGIC_VALUE) {
          fire_index_set(spread_y, spread_x, FIRE_DURATION);
          VERA.address = vera_tilemap_addr_offsets[spread_y][spread_x] + 1;
          VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];
        }
      }
    }
  }
}

// WIND
void wind_setup(void) { wind_direction = rand() % WIND_DIRECTIONS; }
void wind_update(void) {
  // wind_direction = (wind_direction +1) % WIND_DIRECTIONS;
  if (rand() < WIND_CHANGE_CHANCE) {
    wind_direction = wind_direction + ((rand() % 3) - 1);
  }
  if (wind_direction < 0) {
    wind_direction = WIND_DIRECTIONS + wind_direction;
  }
  wind_direction = wind_direction % WIND_DIRECTIONS;
}
void wind_sprites_setup(void) {
  // WIND GAUGE CIRCLE
  VERA.address = WIND_CIRCLE_SPRITE_ATTR_ADDR;
  VERA.address_hi = WIND_CIRCLE_SPRITE_ATTR_ADDR >> 16;
  VERA.address_hi |= VERA_INC_1;

  VERA.data0 = CIRCLE_SPRITE_BITMAP_ADDR >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (CIRCLE_SPRITE_BITMAP_ADDR >> 13);
  VERA.data0 = WIND_GAUGE_X_PX;
  VERA.data0 = WIND_GAUGE_X_PX >> 8;
  VERA.data0 = WIND_GAUGE_Y_PX;
  VERA.data0 = WIND_GAUGE_Y_PX >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2;  // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000;               // 32x32 pixel image
}
void wind_sprite_update(void) {
  VERA.address = WIND_NEEDLE_SPRITE_ATTR_ADDR;
  VERA.address_hi = WIND_NEEDLE_SPRITE_ATTR_ADDR >> 16;
  VERA.address_hi |= VERA_INC_1;

  sprite24_frame(sprite_frame, NEEDLE_SPRITE_BITMAP_ADDR, NEEDLE_SPRITE_FRAME_BYTES, wind_direction);
  VERA.data0 = sprite_frame->frame_addr >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (sprite_frame->frame_addr >> 13);
  VERA.data0 = WIND_GAUGE_X_PX;
  VERA.data0 = WIND_GAUGE_X_PX >> 8;
  VERA.data0 = WIND_GAUGE_Y_PX;
  VERA.data0 = WIND_GAUGE_Y_PX >> 8;
  VERA.data0 = 0b00001100 | sprite_frame->flips;  // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000;                        // 32x32 pixel image
}
void face_sprite_setup(void) {
  VERA.address = FACE_SPRITE_ATTR_ADDR;
  VERA.address_hi = FACE_SPRITE_ATTR_ADDR >> 16;
  VERA.address_hi |= VERA_INC_1;

  VERA.data0 = FACE_SPRITE_BITMAP_ADDR >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (FACE_SPRITE_BITMAP_ADDR >> 13);
  VERA.data0 = 320;
  VERA.data0 = 320 >> 8;
  VERA.data0 = 240;
  VERA.data0 = 240 >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2;  // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = SPRITE_HEIGHT_8PX | SPRITE_WIDTH_8PX;
}

// PEOPLE
void people_setup(unsigned int num_people) {
  unsigned char person_x, person_y;
  unsigned int placed = 0;

  VERA.address_hi = 0 | VERA_INC_1;

  people_count = 0;

  while (placed < num_people) {
    person_x = rand() & 0x3F;  // 0-63
    person_y = rand() & 0x3F;  // 0-63

    // Only place people on non-burning, non-water tiles
    if (fire_index_get(person_y, person_x) < NO_FIRE_MAGIC_VALUE || water_index_get(person_y, person_x) > 0) {
      continue;
    }

    if (people_count < MAX_PEOPLE) {
      // Read and save the current tile character and color
      VERA.address = vera_tilemap_addr_offsets[person_y][person_x];
      people_pool[people_count].backed_char = VERA.data0;
      people_pool[people_count].backed_color = VERA.data0;

      // Write tile 129 at this location
      VERA.address = vera_tilemap_addr_offsets[person_y][person_x];
      VERA.data0 = 0x81;  // tile 129 (0x81 in hex)
      VERA.data0 = 0x21;

      people_pool[people_count].x = person_x;
      people_pool[people_count].y = person_y;
      people_pool[people_count].alive = 1;
      people_count++;
    }

    placed++;
  }
}
void people_move(void) {
  if (people_count == 0)
    return;

  pm_idx = rand() % people_count;
  if (!people_pool[pm_idx].alive)
    return;

  pm_dir = rand() % 8;

  pm_nx = people_pool[pm_idx].x + people_dx[pm_dir];
  pm_ny = people_pool[pm_idx].y + people_dy[pm_dir];

  // bounds check
  if (pm_nx < 0 || pm_nx >= 64 || pm_ny < 0 || pm_ny >= 64)
    return;

  // only move to non-burning, non-water tiles
  if (fire_index_get(pm_ny, pm_nx) < NO_FIRE_MAGIC_VALUE)
    return;
  if (water_index_get(pm_ny, pm_nx) > 0)
    return;

  VERA.address_hi = 0 | VERA_INC_1;

  // restore old position with backed-up tile
  VERA.address = vera_tilemap_addr_offsets[people_pool[pm_idx].y][people_pool[pm_idx].x];
  VERA.data0 = people_pool[pm_idx].backed_char;
  VERA.data0 = people_pool[pm_idx].backed_color;

  // read and save tile at new location before overwriting
  VERA.address = vera_tilemap_addr_offsets[pm_ny][pm_nx];
  people_pool[pm_idx].backed_char = VERA.data0;
  people_pool[pm_idx].backed_color = VERA.data0;

  // draw person at new location
  VERA.address = vera_tilemap_addr_offsets[pm_ny][pm_nx];
  VERA.data0 = 0x81;
  VERA.data0 = 0x21;

  // update pool
  people_pool[pm_idx].x = pm_nx;
  people_pool[pm_idx].y = pm_ny;
}

void draw_map(void) {
  unsigned char r, c;
  VERA.address_hi = 0 | VERA_INC_1;
  for (r = 0; r < 64; r++) {
    for (c = 0; c < 64; c++) {
      VERA.address = vera_tilemap_addr_offsets[r][c];
      VERA.data0 = map_tiles_index_get(r, c, 0) + VALID_MAP_TILES_START_INDEX;
      VERA.data0 = map_tiles_index_get(r, c, 1);
      if (fire_index_get(r, c) < NO_FIRE_MAGIC_VALUE) {
        VERA.address = vera_tilemap_addr_offsets[r][c] + 1;
        VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];
      }
      if (water_index_get(r, c) > 0) {
        VERA.address = vera_tilemap_addr_offsets[r][c] + 1;
        VERA.data0 = 0x66;
      }
    }
  }
}

// EXECUTION
void outro(clock_t start_time, clock_t end_time) {
  unsigned long fps = 0;
  unsigned long runtime_seconds = 0;

  runtime_seconds = 1 + ((end_time - start_time) / CLOCKS_PER_SEC);
  fps = game_frame / runtime_seconds;

  // Reset VERA to text mode
  VERA.display.video = 0b00100001;  // Reset to text mode with only layer 1 active

  // Reset layer 0 to default text mode configuration
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
  VERA.layer1.config = 0b01100000;  // 128x64
  VERA.layer1.mapbase = (0x1b000 >> 9) & 0b11111100;

  // petsci lower / gfx 80x30
  asm("lda #2");
  asm("jsr $FF62");
  videomode(3);

  printf("end of game");

  printf("\n\nframes: %lu", game_frame);
  printf("\nruntime: %lu seconds", runtime_seconds);
  printf("\nfps: %lu\n\n", fps);
}
void main(void) {
  bool run = true;
  unsigned char joy;
  clock_t start_time;
  clock_t end_time;

  uppercase_petscii_40x30();
  random_setup();
  vera_loads();
  vera_tilemap_addr_offsets_setup();
  joy_install(cx16_std_joy);
  fire_color_setup();
  map_setup();
  fire_setup();
  bombs_setup();
  draw_map();
  people_setup(100);  // Place 100 people on the map
  wind_setup();
  wind_sprites_setup();
  vera_screen_setup();
  // face_sprite_setup();
  // mouse_setup();

  start_time = clock();

  while (run) {
    joy = joy_read(0);

    if (JOY_DOWN(joy)) {
      run = false;
    }
    game_frame++;
    burn();
    wind_update();
    wind_sprite_update();
    bombs_blink();
    bombs_explode();
    people_move();
    wait();
  }

  end_time = clock();
  outro(start_time, end_time);
}