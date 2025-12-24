#ifndef FIRE_H
#define FIRE_H

#include <stdbool.h>

// Forward declarations
void detonate_bomb(unsigned char b);

// HEADER OPTIONS
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

// FIRE DYNAMICS
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

#define BOMB_BANK 2
#define BOMB_DATA_ADDRESS 0xA000  // banked ram window

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

// BOMBS
#define NUM_BOMBS 16
#define NUM_UXBOMB_FRAMES 4
#define BOMB_BURNING_DETONATION_CHANCE 16000
#define EXPLOSION_FRAMES 7
#define EXPLOSION_SIZE_TILES 9
#define EXPLOSION_IGNITE_CHANCE 3000
#define EXPLOSION_FLATTEN_CHANCE 16000
#define EXPLOSION_BURNOUT_CHANCE 16000
#define EXPLOSION_DETONATE_CHANCE 30000

// PEOPLE
#define MAX_PEOPLE 50

// TYPE DEFINITIONS
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

typedef struct { 
  unsigned char x; 
  unsigned char y; 
  unsigned char alive; 
} Person;

// GLOBAL VARIABLE DECLARATIONS
extern unsigned char fire_bgcolors[FIRE_NUM_BG_COLORS];
extern unsigned char fire_fgcolors[FIRE_NUM_FG_COLORS];
extern unsigned char fire_colors[FIRE_COLOR_COMBINATIONS];
extern char (*fire_index)[64];
extern char (*water_index)[64];
extern char (*bomb_index)[64];
extern unsigned char bomb_colors[NUM_UXBOMB_FRAMES];
extern unsigned char bomb_chars[NUM_UXBOMB_FRAMES];
extern unsigned char bomb_explosion_animation[EXPLOSION_FRAMES][EXPLOSION_SIZE_TILES][EXPLOSION_SIZE_TILES];
extern Person people_pool[MAX_PEOPLE];
extern unsigned int people_count;
extern const signed char people_dx[8];
extern const signed char people_dy[8];
extern unsigned int pm_idx;
extern signed char pm_dir;
extern signed char pm_nx;
extern signed char pm_ny;
extern SpriteFrame sprite_frame_data;
extern SpriteFrame* sprite_frame;
extern Bomb bomb_pool[NUM_BOMBS];
extern unsigned long game_frame;
extern unsigned char areg;
extern unsigned char rand_x, rand_y;
extern unsigned long addr;
extern unsigned int sample;
extern unsigned int vera_tilemap_addr_offsets[64][64];
extern const signed char fire_xpread[8];
extern const signed char fire_ypread[8];
extern signed char dieroll;
extern signed char spread_x, spread_y, keycode;
extern unsigned char file_error_num;
extern unsigned int die_roll;
extern signed char wind_direction;
extern signed char needle_sprite_frame;

#endif // FIRE_H
