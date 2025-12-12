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

#define SKIP_2_BYTE_HEADER 0
#define USE_2_BYTE_HEADER 1
#define NO_2_BYTE_HEADER 2

// VERA LOAD ADDRESSES
#define MAP0_BASE_ADDR 0x0000
#define MAP1_BASE_ADDR 0x2000
#define NEEDLE_SPRITE_BASE_ADDR 0x6000
#define CIRCLE_SPRITE_BASE_ADDR 0x6E00
#define CHARSET_BASE_ADDR 0x1F000
#define SPRITE_ATTR_BASE_ADDR 0x1FC08

#define MONOPLANE_SPRITE_BASE_ADDR 0x17800
#define FLAK_SPRITE_BASE_ADDR 0x17B80
#define CROSSHAIR_SPRITE_BASE_ADDR 0x17F00
#define FLAK_BURST_SPRITE_BASE_ADDR 0x18100
#define FLAK_SHELL_SPRITE_BASE_ADDR 0x18F00
#define PALETTE_BASE_ADDR 0x1FA00

#define HI_RES true

#define MAP_WIDTH_TILES 64

// FIRE APPEARANCE
#define FIRE_NUM_BG_COLORS 4
#define FIRE_NUM_FG_COLORS 8
#define FIRE_COLOR_COMBINATIONS (FIRE_NUM_FG_COLORS * FIRE_NUM_BG_COLORS)
unsigned char fire_bgcolors[FIRE_NUM_BG_COLORS] = {0x20, 0x70, 0x80, 0xA0};
unsigned char fire_fgcolors[FIRE_NUM_FG_COLORS] = {0x01, 0x02, 0x03,0x07, 0x08, 0x0A, 0x0D, 0x0F};
unsigned char fire_colors[FIRE_COLOR_COMBINATIONS];

// fire dynamics
#define FIRE_SAMPLES_PER_FRAME 72
#define FIRE_DURATION 50
#define NO_FIRE_MAGIC_VALUE (FIRE_DURATION+1)
#define FIRE_SEED_CHANCE 100
#define FIRE_SPREAD_CHANCE 16000
#define SOAKED_SEED_CHANCE 500
#define SOAK_DURATION 10
#define BURNT_OUT 255

//FIRE MEMORY
#define BANK_NUM (*(unsigned char *)0x00)
#define FIRE_DATA_ADDRESS 0xA000   // banked ram window
#define WATER_DATA_ADDRESS 0xB000 
char (*fire_data)[64] = (char (*)[64])FIRE_DATA_ADDRESS;
char (*water_data)[64] = (char (*)[64])WATER_DATA_ADDRESS;

// WIND
#define WIND_CHANGE_CHANCE 3000
#define WIND_DIRECTIONS 24
#define WIND_GAUGE_X_PX 600
#define WIND_GAUGE_Y_PX 440

// SPRITE INDICES
// these are base 0 from 1FC08, 
//leaving room at 1FC00 for 
// the mouse cursor sprite
#define SPRITE_DEF_SIZE_BYTES 8
#define NEEDLE_SPRITE_FRAME_BYTES 512
#define WIND_GAUGE_NEEDLE_SPRITE_ADDR (SPRITE_ATTR_BASE_ADDR + (0 * SPRITE_DEF_SIZE_BYTES))
#define WIND_GAUGE_CIRCLE_SPRITE_ADDR (SPRITE_ATTR_BASE_ADDR + (1 * SPRITE_DEF_SIZE_BYTES))

typedef struct {
  unsigned char flips;
  unsigned long frame_addr;
} SpriteFrame;

unsigned char joy;
unsigned long game_frame = 0;
clock_t start_time;
clock_t end_time;
unsigned long runtime_seconds;
unsigned char areg;
unsigned char rand_x, rand_y;
unsigned long addr;
unsigned int sample;
unsigned int vera_tilemap_addr_offsets[64][64];
signed char fire_xpread[8] = {-1,0,1,-1,1,-1,0,1};
signed char fire_ypread[8] = {1,1,1,0,0,-1,-1,-1};
signed char dieroll;
unsigned char spread_x, spread_y, keycode;
unsigned char file_error_num = 0;
unsigned int die_roll;  /// ugh
signed char wind_direction = 0;
signed char needle_sprite_frame;
SpriteFrame* sprite_frame;

void setup_random(void) {
  // call entropy_get to seed the random number generator
  asm("jsr $FECF");
  asm("STA %v", areg);  // Added missing '&' for address reference
  srand(areg);
}

// VERA
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

  cbm_k_setlfs(SKIP_2_BYTE_HEADER, 8, secondary_address);

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
    printf("%1c%1c err#%02u%1c\n",28,0x71,file_error_num,5);
    exit(1);
  }
  else {
    printf("%1c%1c%1c\n", 30, 0x73, 5);
  }
}
void vera_loads(void) {
  load_into_vera("map0.bin", MAP0_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("map1.bin", MAP1_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("sprite1.bin", NEEDLE_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("circle.bin", CIRCLE_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  // load_into_vera("sprite0.bin", SHIP_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  // load_into_vera("monoplane16.bin", MONOPLANE_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("flak16.bin", FLAK_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("crosshair32.bin", CROSSHAIR_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("flakburst32.bin", FLAK_BURST_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("flakshell16.bin", FLAK_SHELL_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  // load_into_vera("palette.bin", PALETTE_BASE_ADDR, NO_2_BYTE_HEADER);
}
void vera_setup(void) {

  // petsci upper / gfx
  asm("lda #2");
  asm("jsr $FF62");

  VERA.display.video = 0b01110001;    // activate layers & sprites
  VERA.display.hscale = HI_RES ? 128 : 64;
  VERA.display.vscale = HI_RES ? 128 : 64;

  VERA.layer0.mapbase = (MAP0_BASE_ADDR >> 9) & 0xFF;  // top eight bits of 17-bit address and 16x16

  VERA.layer0.config = 
    LAYER_MAP_HEIGHT_64 | 
    LAYER_MAP_WIDTH_64 | 
    LAYER_T256C_OFF | 
    LAYER_BITMAP_OFF | 
    LAYER_BPP_1;
  VERA.layer0.tilebase =
    (CHARSET_BASE_ADDR >> 9)  // top six bits of 17-bit address 
    & 0b11111100;             // tile height / width = 8px

  VERA.layer1.config = 0b01100000;  // 128(w)x64(h) 16-color tiles
  VERA.layer1.mapbase = (MAP1_BASE_ADDR >> 9) & 0b11111100;  // top eight bits of 17-bit address and 8x8
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
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
      fire_colors[lookup_index] = (fire_bgcolors[bg_index] | fire_fgcolors[fg_index]);
      lookup_index++;
    }
  }
}
void fire_setup(void) {
  unsigned char c, r;
  unsigned long addr;
  unsigned char neighbor_count,pass;
  
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
        fire_data[r][c] = FIRE_DURATION;
        water_data[r][c] = 0;
      } else if (die_roll < SOAKED_SEED_CHANCE) {
        water_data[r][c] = SOAK_DURATION;
      } else {
        fire_data[r][c] = NO_FIRE_MAGIC_VALUE;
        water_data[r][c] = 0;
      }
    }
  }
  for (pass = 0; pass < 3; pass++) {
    for (r = 1; r < 63; r++) {  // Skip edges to avoid boundary checks
      for (c = 1; c < 63; c++) {
        if (water_data[r][c] == 0) { 
          neighbor_count = 0;
          
          // Count neighbors (8-connected)
          if (water_data[r-1][c-1] > 0) neighbor_count++;
          if (water_data[r-1][c]   > 0) neighbor_count++;
          if (water_data[r-1][c+1] > 0) neighbor_count++;
          if (water_data[r][c-1]   > 0) neighbor_count++;
          if (water_data[r][c+1]   > 0) neighbor_count++;
          if (water_data[r+1][c-1] > 0) neighbor_count++;
          if (water_data[r+1][c]   > 0) neighbor_count++;
          if (water_data[r+1][c+1]  > 0) neighbor_count++;

          // Higher probability based on neighbor count
          if (neighbor_count > 0) {
            unsigned int threshold = neighbor_count * 4800;
            if (rand() < threshold) {
              water_data[r][c] = SOAK_DURATION;
            }
          }
        }
      }
    }
  }

  for (pass = 0; pass < 3; pass++) {
    for (r = 1; r < 63; r++) {  // Skip edges to avoid boundary checks
      for (c = 1; c < 63; c++) {
        if (fire_data[r][c] == NO_FIRE_MAGIC_VALUE && water_data[r][c] == 0) { 
          neighbor_count = 0;
          
          // Count neighbors (8-connected)
          if (fire_data[r-1][c-1] < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r-1][c]   < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r-1][c+1] < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r][c-1]   < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r][c+1]   < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r+1][c-1] < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r+1][c]   < NO_FIRE_MAGIC_VALUE) neighbor_count++;
          if (fire_data[r+1][c+1] < NO_FIRE_MAGIC_VALUE) neighbor_count++;

          // Higher probability based on neighbor count
          if (neighbor_count > 0) {
            unsigned int threshold = neighbor_count * 4500;
            if (rand() < threshold) {
              fire_data[r][c] = FIRE_DURATION;
            }
          }
        }
      }
    }
  }

  VERA.address_hi = 0;

  for (r = 0; r < 64; r++) {
    for (c = 0; c < 64; c++) {
      if (fire_data[r][c] < NO_FIRE_MAGIC_VALUE) {
        addr = vera_tilemap_addr_offsets[r][c];
        VERA.address = addr;
        VERA.data0 = fire_colors[rand() % FIRE_COLOR_COMBINATIONS];
      }
      if (water_data[r][c] > 0) {
        addr = vera_tilemap_addr_offsets[r][c];
        VERA.address = addr;
        VERA.data0 = 0x60;
      }
    }
  }
}
void burn() {

  VERA.address_hi = 0 | VERA_INC_1;

  for (sample = 0; sample < FIRE_SAMPLES_PER_FRAME; sample++) {
    rand_x = rand() & 0x3F;  // 0-63 (mask with 0011 1111)
    rand_y = rand() & 0x3F;  // 0-63 (mask with 0011 1111)

    // 
    // YOU DO NOT BURN
    //
    if (fire_data[rand_y][rand_x] > FIRE_DURATION) {
      continue;
    }

    // 
    // YOU BURN 
    //
    fire_data[rand_y][rand_x]--;

    // 
    // YOU BURN OUT
    //
    if (fire_data[rand_y][rand_x] == 0) {
      fire_data[rand_y][rand_x] = BURNT_OUT;
      if ((rand() % 3) == 0) {
        VERA.address = vera_tilemap_addr_offsets[rand_y][rand_x]-1;
        VERA.data0 = 0x66;
      }
      else {
        VERA.address = vera_tilemap_addr_offsets[rand_y][rand_x];
      }
      VERA.data0 = 0xB0;      
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
      if (spread_x >= 0 && spread_x < 64 && spread_y>=0 && spread_y < 64) {
        if (water_data[spread_y][spread_x] > 0) {
          water_data[spread_y][spread_x]--;
        }
        else if (fire_data[spread_y][spread_x] == NO_FIRE_MAGIC_VALUE) {
          fire_data[spread_y][spread_x] = FIRE_DURATION;
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
  VERA.address = WIND_GAUGE_CIRCLE_SPRITE_ADDR;
  VERA.address_hi = WIND_GAUGE_CIRCLE_SPRITE_ADDR >> 16;
  VERA.address_hi |= VERA_INC_1;

  VERA.data0 = CIRCLE_SPRITE_BASE_ADDR >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (CIRCLE_SPRITE_BASE_ADDR >> 13);
  VERA.data0 = WIND_GAUGE_X_PX;
  VERA.data0 = WIND_GAUGE_X_PX >> 8;
  VERA.data0 = WIND_GAUGE_Y_PX;
  VERA.data0 = WIND_GAUGE_Y_PX >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000; // 32x32 pixel image
}
void wind_sprite_update(void) {

  VERA.address = WIND_GAUGE_NEEDLE_SPRITE_ADDR;
  VERA.address_hi = WIND_GAUGE_NEEDLE_SPRITE_ADDR >> 16;
  VERA.address_hi |= VERA_INC_1;

  needle_sprite_frame = wind_direction;
  sprite24_frame(sprite_frame, NEEDLE_SPRITE_BASE_ADDR, NEEDLE_SPRITE_FRAME_BYTES, needle_sprite_frame);
  VERA.data0 = sprite_frame->frame_addr >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (sprite_frame->frame_addr >> 13);
  VERA.data0 = WIND_GAUGE_X_PX;
  VERA.data0 = WIND_GAUGE_X_PX >> 8;
  VERA.data0 = WIND_GAUGE_Y_PX;
  VERA.data0 = WIND_GAUGE_Y_PX >> 8;
  VERA.data0 = 0b00001100 | sprite_frame->flips; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000; // 32x32 pixel image
}

// EXECUTION
void outro(void) {
  unsigned long fps = 0;

  end_time = clock();
  runtime_seconds = 1+ ((end_time - start_time) / CLOCKS_PER_SEC);
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

  setup_random();

  asm("lda #2");
  asm("jsr $FF62");
  videomode(3);

  asm("sec");
  asm("jsr $FF5F");
  asm("lda #1");
  asm("jsr $FF68");   // Call mouse_config Kernal Function
  wait();             // Wait a cycle for the mouse to fully activate

  printf("loading vera\n");
  vera_loads();
  joy_install(cx16_std_joy);
  printf("\nrandomizing fire");
  fire_color_setup();
  fire_setup();
  wind_setup();
  vera_setup();
  wind_sprites_setup();

  //mousey
  asm("ldx #80");
  asm("ldy #60");
  asm("lda #1");
  asm("jsr $FF68");   // Call mouse_config Kernal Function
  wait();             // Wait a cycle for the mouse to fully activate

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
    wait(); 
  }

  outro();

}