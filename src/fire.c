#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdbool.h>
#include <joystick.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "wait.h"
#include "vera.h"

// Forward declarations
void detonate_bomb(unsigned char b);

#define SKIP_2_BYTE_HEADER 0
#define USE_2_BYTE_HEADER 1
#define NO_2_BYTE_HEADER 2

// VERA LOAD ADDRESSES
#define MAP0_ADDR 0x0000
#define MAP1_ADDR 0x2000
#define NEEDLE_SPRITE_BITMAP_ADDR 0x6000
#define CIRCLE_SPRITE_BITMAP_ADDR 0x6E00
#define FACE_SPRITE_BITMAP_ADDR 0x7000
#define TILESET_ADDR 0x1F000
#define SPRITE_ATTR_ADDR 0x1FC08

#define MAP_WIDTH_TILES 64

// FIRE APPEARANCE
#define FIRE_NUM_BG_COLORS 4
#define FIRE_NUM_FG_COLORS 2
#define FIRE_COLOR_COMBINATIONS 18
unsigned char fire_bgcolors[FIRE_NUM_BG_COLORS] = { 0x20, 0x70, 0x80, 0xA0 };
unsigned char fire_fgcolors[FIRE_NUM_FG_COLORS] = { 0x00, 0x01 };
unsigned char fire_colors[FIRE_COLOR_COMBINATIONS] = 
{0x22, 0x77, 0x88, 0XAA, 0x22, 0x77, 0x88, 0XAA, 0x77, 0x87, 0XA7, 0x71, 0x81, 0xA1, 0x77, 0x88, 0x77, 0x88};

// fire dynamics
#define FIRE_SAMPLES_PER_FRAME 72
#define FIRE_DURATION 18
#define NO_FIRE_MAGIC_VALUE (FIRE_DURATION+1)
#define FIRE_SEED_CHANCE 100
#define FIRE_SPREAD_CHANCE 12000
#define SOAKED_SEED_CHANCE 500
#define SOAK_DURATION 10
#define BURNT_OUT 255
#define BURNT_OUT_CRUMBLE_TILE_CHANCE 12000

// BANKED RAM
#define BANK_NUM (*(unsigned char *)0x00)
#define FIRE_AND_WATER_BANK 1
#define FIRE_DATA_ADDRESS 0xA000   // banked ram window
#define WATER_DATA_ADDRESS 0xB000  // top half of banked ram window
char (*fire_index)[64] = (char (*)[64])FIRE_DATA_ADDRESS;
char (*water_index)[64] = (char (*)[64])WATER_DATA_ADDRESS;

#define BOMB_BANK 2
#define BOMB_DATA_ADDRESS 0xA000  // banked ram window
char (*bomb_index)[64] = (char (*)[64])BOMB_DATA_ADDRESS;  // must use BANK_NUM=2

// WIND
#define WIND_CHANGE_CHANCE 750
#define WIND_DIRECTIONS 24
#define WIND_GAUGE_X_PX 600
#define WIND_GAUGE_Y_PX 440

// SPRITE INDICES
#define SPRITE_DEF_SIZE_BYTES 8
#define NEEDLE_SPRITE_FRAME_BYTES 512
#define WIND_NEEDLE_SPRITE_ATTR_ADDR (SPRITE_ATTR_ADDR + (0 * SPRITE_DEF_SIZE_BYTES))
#define WIND_CIRCLE_SPRITE_ATTR_ADDR (SPRITE_ATTR_ADDR + (1 * SPRITE_DEF_SIZE_BYTES))
#define FACE_SPRITE_ATTR_ADDR        (SPRITE_ATTR_ADDR + (2 * SPRITE_DEF_SIZE_BYTES))

//BOMBS
#define NUM_BOMBS 16
#define NUM_UXBOMB_FRAMES 4
#define BOMB_BURNING_DETONATION_CHANCE 16000
#define EXPLOSION_FRAMES 7
#define EXPLOSION_SIZE_TILES 9
#define EXPLOSION_IGNITE_CHANCE 3000
#define EXPLOSION_FLATTEN_CHANCE 16000
#define EXPLOSION_BURNOUT_CHANCE 16000
#define EXPLOSION_DETONATE_CHANCE 30000
unsigned char bomb_colors[NUM_UXBOMB_FRAMES] = { 0x00, 0x01, 0x00, 0x03 };
unsigned char bomb_chars[NUM_UXBOMB_FRAMES] = { 0x00, 0x57, 0x00, 0x5A };
unsigned char bomb_explosion_animation[EXPLOSION_FRAMES][EXPLOSION_SIZE_TILES][EXPLOSION_SIZE_TILES] = {
  {  // FRAME 0
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0}
  },
  {  // FRAME 1
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,0,0,0,0},
    {0,0,0,1,0,1,0,0,0},
    {0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0}
  },
  {  // FRAME 2
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,0,0,0,0},
    {0,0,0,1,0,1,0,0,0},
    {0,0,1,0,0,0,1,0,0},
    {0,0,0,1,0,1,0,0,0},
    {0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0}
  },
  {  // FRAME 3
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,0,0,0},
    {0,0,1,0,0,0,1,0,0},
    {0,0,1,0,0,0,1,0,0},
    {0,0,1,0,0,0,1,0,0},
    {0,0,0,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0}
  },
  {  // FRAME 4
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,0,0,0},
    {0,0,1,0,0,0,1,0,0},
    {0,1,0,0,0,0,0,1,0},
    {0,1,0,0,0,0,0,1,0},
    {0,1,0,0,0,0,0,1,0},
    {0,0,1,0,0,0,1,0,0},
    {0,0,0,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,0}
  },
  {  // FRAME 5
    {0,0,1,1,1,1,1,0,0},
    {0,1,0,0,0,0,0,1,0},
    {1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1},
    {0,1,0,0,0,0,0,1,0},
    {0,0,1,1,1,1,1,0,0}
  }
};

typedef struct {
  unsigned char flips;
  unsigned long frame_addr;
} SpriteFrame;

typedef struct {
  unsigned char frame;
  unsigned char x;
  unsigned char y;
  bool exploding;
  bool unexploded;
} Bomb;

#define MAX_PEOPLE 4096

typedef struct { unsigned char x; unsigned char y; unsigned char alive; } Person;

Person people_pool[MAX_PEOPLE];
unsigned int people_count = 0;

const signed char people_dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
const signed char people_dy[8] = {  1, 1, 1,  0, 0, -1, -1, -1 };

/* globals used by people_move to avoid function-local variables */
unsigned int pm_idx;
signed char pm_dir;
signed char pm_nx;
signed char pm_ny;

SpriteFrame sprite_frame_data = { 0, 0 };
SpriteFrame* sprite_frame = &sprite_frame_data;
Bomb bomb_pool[NUM_BOMBS];

unsigned long game_frame = 0;
unsigned char areg;
unsigned char rand_x, rand_y;
unsigned long addr;
unsigned int sample;
unsigned int vera_tilemap_addr_offsets[64][64];
const signed char fire_xpread[8] = { -1,0,1,-1,1,-1,0,1 };
const signed char fire_ypread[8] = { 1,1,1,0,0,-1,-1,-1 };
signed char dieroll;
signed char spread_x, spread_y, keycode;
unsigned char file_error_num = 0;
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
  asm("jsr $FF68");   // Call mouse_config Kernal Function
  wait();             // Wait a cycle for the mouse to fully activate
}

// VERA
void uppercase_petscii_40x30(void) {
  asm("lda #2");
  asm("jsr $FF62");
  videomode(3);
}
void load_into_vera(char* filename, unsigned long base_addr, char secondary_address) {

  unsigned char m = 2;

  // These 3 functions are basic wrappers for the Kernal Functions

  // You have to first set the name of the file you are working with.
  cbm_k_setnam(filename);
  printf("  %-15s $%05lx ", filename, base_addr);

  // Next you setup the LFS (Logical File) for the file
  // First param is the Logical File Number
  //   Use 0 if you are just loading the file
  //   You can use other values to keep multiple files open
  // Second param is the device number
  //   The SD Card on the CX16 is 8
  // The last param is the Secondary Address
  // 0 - File has the 2 byte header, but skip it
  // 1 - File has the 2 byte header, use it
  // 2 - File does NOT have the 2 byte header

  cbm_k_setlfs(0, 8, secondary_address);

  if (base_addr >= 0x10000) {
    base_addr -= 0x10000;
    m = 3;
  }

  // // Finally, load the file somewhere into RAM or VRAM
  // // First param of cbm_k_load means:
  // //   0, loads into system memory.
  // //   1, perform a verify.
  // //   2, loads into VRAM, starting from 0x00000 + the specified starting address.
  // //   3, loads into VRAM, starting from 0x10000 + the specified starting address.
  // // Second param is the 16 bit address 
  cbm_k_load(m, base_addr);

  file_error_num = 0;
  __asm__("bcc noerror");
  __asm__("lda #1");
  __asm__("sta %v", file_error_num);
  __asm__("noerror:");

  if (file_error_num) {
    printf("%1c%1c err#%02u%1c\n", 28, 0x71, file_error_num, 5);
    exit(1);
  }
  else {
    printf("%1c%1c%1c\n", 30, 0x73, 5);
  }
}
void vera_loads(void) {
  printf("loading vera\n");
  load_into_vera("map0.bin", MAP0_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("map1.bin", MAP1_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("sprite1.bin", NEEDLE_SPRITE_BITMAP_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("circle.bin", CIRCLE_SPRITE_BITMAP_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("face.bin", FACE_SPRITE_BITMAP_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("petscii.bin", TILESET_ADDR, SKIP_2_BYTE_HEADER);

  // load_into_vera("sprite0.bin", SHIP_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  // load_into_vera("monoplane16.bin", MONOPLANE_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("flak16.bin", FLAK_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("crosshair32.bin", CROSSHAIR_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("flakburst32.bin", FLAK_BURST_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("flakshell16.bin", FLAK_SHELL_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("palette.bin", PALETTE_BASE_ADDR, NO_2_BYTE_HEADER);
}
void vera_screen_setup(void) {

  VERA.display.video = 
    SPRITES_ENABLED |
    LAYER1_ENABLED |
    LAYER0_ENABLED |
    VGA_ENABLED;

  VERA.display.hscale = DC_HSCALE_640;
  VERA.display.vscale = DC_VSCALE_480;

  VERA.layer0.mapbase = (MAP0_ADDR >> 9);

  VERA.layer0.config =
    LAYER_MAP_HEIGHT_64 |
    LAYER_MAP_WIDTH_64 |
    LAYER_T256C_OFF |
    LAYER_BITMAP_OFF |
    LAYER_BPP_1;

  VERA.layer0.tilebase = 
    (TILESET_ADDR >> 9 & TILE_BASE_ADDR_MASK) | 
    TILE_HEIGHT_8PX | 
    TILE_WIDTH_8PX;

  VERA.layer1.config = 
    LAYER_MAP_HEIGHT_64 | 
    LAYER_MAP_WIDTH_128 | 
    LAYER_T256C_OFF | 
    LAYER_BITMAP_OFF | 
    LAYER_BPP_1;

  VERA.layer1.tilebase = 
    (TILESET_ADDR >> 9 & TILE_BASE_ADDR_MASK) | 
    TILE_HEIGHT_8PX | 
    TILE_WIDTH_8PX;

  VERA.layer1.mapbase = (MAP1_ADDR >> 9); 
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
}

// BANKED SHIT
unsigned char bomb_index_get(unsigned char row, unsigned char col) {
  BANK_NUM = BOMB_BANK;
  return bomb_index[row][col];
}
void bomb_index_set(unsigned char row, unsigned char col, unsigned char value) {
  BANK_NUM = BOMB_BANK;
  bomb_index[row][col] = value;
}
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

// SPRITES
void sprite24_frame(SpriteFrame* sf, unsigned long base_addr, unsigned int frame_size_bytes, unsigned char frame) {
  if (frame >= 19) {
    sf->flips = 0b01;
    sf->frame_addr = base_addr + (frame_size_bytes * (24 - frame));
  }
  else if (frame >= 13) {
    sf->flips = 0b11;
    sf->frame_addr = base_addr + (frame_size_bytes * (frame - 12));
  }
  else if (frame >= 7) {
    sf->flips = 0b10;
    sf->frame_addr = base_addr + (frame_size_bytes * (12 - frame));
  }
  else {
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
void fire_setup(void) {
  unsigned char c, r;
  unsigned long addr;
  unsigned char neighbor_count, pass;
  unsigned int tiles_processed = 0;

  // Precompute all address offsets
  for (r = 0; r < 64; r++) {
    for (c = 0; c < 64; c++) {
      vera_tilemap_addr_offsets[r][c] = 1 + (2 * (r * MAP_WIDTH_TILES + c));
    }
  }

  // Initial random seeding - lower probability
  for (r = 0; r < 64; r++) {
    for (c = 0; c < 64; c++) {
      die_roll = rand();
      if (die_roll < FIRE_SEED_CHANCE) {
        fire_index_set(r, c, FIRE_DURATION);
        water_index_set(r, c, 0);
      }
      else if (die_roll < SOAKED_SEED_CHANCE) {
        water_index_set(r, c, SOAK_DURATION);
      }
      else {
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
          if (water_index_get(r - 1, c - 1) > 0) neighbor_count++;
          if (water_index_get(r - 1, c) > 0) neighbor_count++;
          if (water_index_get(r - 1, c + 1) > 0) neighbor_count++;
          if (water_index_get(r, c - 1) > 0) neighbor_count++;
          if (water_index_get(r, c + 1) > 0) neighbor_count++;
          if (water_index_get(r + 1, c - 1) > 0) neighbor_count++;
          if (water_index_get(r + 1, c) > 0) neighbor_count++;
          if (water_index_get(r + 1, c + 1) > 0) neighbor_count++;

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
          if (fire_index_get(r - 1, c - 1) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r - 1, c) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r - 1, c + 1) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r, c - 1) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r, c + 1) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r + 1, c - 1) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r + 1, c) < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_index_get(r + 1, c + 1) < NO_FIRE_MAGIC_VALUE) neighbor_count++;

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

  VERA.address_hi = 0;

  for (r = 0; r < 64; r++) {
    for (c = 0; c < 64; c++) {
      if (fire_index_get(r, c) < NO_FIRE_MAGIC_VALUE) {
        addr = vera_tilemap_addr_offsets[r][c];
        VERA.address = addr;
        VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];
      }
      if (water_index_get(r, c) > 0) {
        VERA.address = vera_tilemap_addr_offsets[r][c];
        VERA.data0 = 0x60;
      }
    }
  }
}
void burn_out_tile(unsigned char y, unsigned char x) {
  fire_index_set(y, x, BURNT_OUT);
  if ((rand() < BURNT_OUT_CRUMBLE_TILE_CHANCE)) {
    VERA.address = vera_tilemap_addr_offsets[y][x] - 1;
    VERA.data0 = 0x66;
  }
  else {
    VERA.address = vera_tilemap_addr_offsets[y][x];
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
    VERA.address = vera_tilemap_addr_offsets[rand_y][rand_x];
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
        }
        else if (fire_index_get(spread_y, spread_x) == NO_FIRE_MAGIC_VALUE) {
          fire_index_set(spread_y, spread_x, FIRE_DURATION);
          VERA.address = vera_tilemap_addr_offsets[spread_y][spread_x];
          VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];
        }
      }
    }
  }
}

// WIND
void wind_setup(void) {
  wind_direction = rand() % WIND_DIRECTIONS;
}
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
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000; // 32x32 pixel image
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
  VERA.data0 = 0b00001100 | sprite_frame->flips; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000; // 32x32 pixel image
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
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = SPRITE_HEIGHT_8PX | SPRITE_WIDTH_8PX;
}

// BOMBS
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

    VERA.address = vera_tilemap_addr_offsets[bomb_y][bomb_x] - 1;
    VERA.data0 = bomb_chars[0];
    VERA.data0 = bomb_colors[0];
  }
}
void bombs_blink(void) {
  unsigned char b = (game_frame % NUM_BOMBS);
  VERA.address_hi = 0 | VERA_INC_1;

  if (bomb_pool[b].unexploded) {
    VERA.address = vera_tilemap_addr_offsets[bomb_pool[b].y][bomb_pool[b].x] - 1;
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
          VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x];
          VERA.data0 = 0x11;
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
              VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x] - 1;
              VERA.data0 = 0x66;
            }
          }

          // sometimes ignite non-burning land cells
          if (fire_index_get(exp_y, exp_x) == NO_FIRE_MAGIC_VALUE) {
            if (rand() < EXPLOSION_IGNITE_CHANCE) {
              fire_index_set(exp_y, exp_x, FIRE_DURATION);
              VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x];
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
          VERA.address = vera_tilemap_addr_offsets[exp_y][exp_x];
          VERA.data0 = 0x90;
        }
      }
    }
    bomb_pool[b].frame++;
  }
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

    // Write tile 129 at this location
    VERA.address = vera_tilemap_addr_offsets[person_y][person_x] -1;
    VERA.data0 = 0x81;  // tile 129 (0x81 in hex)
    VERA.data0 = 0x21;  

    if (people_count < MAX_PEOPLE) {
      people_pool[people_count].x = person_x;
      people_pool[people_count].y = person_y;
      people_pool[people_count].alive = 1;
      people_count++;
    }

    placed++;
  }
}

void people_move(void) {
  if (people_count == 0) return;

  pm_idx = rand() % people_count;
  if (!people_pool[pm_idx].alive) return;

  pm_dir = rand() % 8;

  pm_nx = people_pool[pm_idx].x + people_dx[pm_dir];
  pm_ny = people_pool[pm_idx].y + people_dy[pm_dir];

  // bounds check
  if (pm_nx < 0 || pm_nx >= 64 || pm_ny < 0 || pm_ny >= 64) return;

  // only move to non-burning, non-water tiles
  if (fire_index_get(pm_ny, pm_nx) < NO_FIRE_MAGIC_VALUE) return;
  if (water_index_get(pm_ny, pm_nx) > 0) return;

  VERA.address_hi = 0 | VERA_INC_1;

  // clear old position (paint generic land)
  VERA.address = vera_tilemap_addr_offsets[people_pool[pm_idx].y][people_pool[pm_idx].x] - 1;
  VERA.data0 = 0x90; // generic land tile char
  VERA.data0 = 0x01; // generic color

  // draw person at new location
  VERA.address = vera_tilemap_addr_offsets[pm_ny][pm_nx] - 1;
  VERA.data0 = 0x81;
  VERA.data0 = 0x21;

  // update pool
  people_pool[pm_idx].x = pm_nx;
  people_pool[pm_idx].y = pm_ny;
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

  //petsci lower / gfx 80x30
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
  joy_install(cx16_std_joy);
  fire_color_setup();
  fire_setup();
  bombs_setup();
  people_setup(100);  // Place 100 people on the map
  wind_setup();
  wind_sprites_setup();
  vera_screen_setup();
  face_sprite_setup();
  mouse_setup();

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