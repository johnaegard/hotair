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

// Uncomment the following line to enable debug console mode 
// #define DEBUG_CONSOLE

#define SKIP_2_BYTE_HEADER 0
#define USE_2_BYTE_HEADER 1
#define NO_2_BYTE_HEADER 2

#define NUM_SHIP_BEARINGS 72
#define DEGREES_PER_FACING 360/NUM_SHIP_BEARINGS

#define MAP0_BASE_ADDR 0x00000
#define SHIP_SPRITE_BASE_ADDR 0x10000
#define MAP1_BASE_ADDR 0x12800
#define NEEDLE_SPRITE_BASE_ADDR 0x16800
#define CIRCLE_SPRITE_BASE_ADDR 0x17600
#define MONOPLANE_SPRITE_BASE_ADDR 0x17800
#define FLAK_SPRITE_BASE_ADDR 0x17B80
#define CROSSHAIR_SPRITE_BASE_ADDR 0x17F00
#define FLAK_BURST_SPRITE_BASE_ADDR 0x18100
#define FLAK_SHELL_SPRITE_BASE_ADDR 0x18F00
#define SPRITE_ATTR_BASE_ADDR 0x1FC08
#define CHARSET_BASE_ADDR 0x1F000
#define PALETTE_BASE_ADDR 0x1FA00

#define SHIP_SPRITE_FRAME_BYTES 512
#define NEEDLE_SPRITE_FRAME_BYTES 512
#define MONOPLANE_SPRITE_FRAME_BYTES 128
#define FLAK_SPRITE_FRAME_BYTES 128
#define FLAK_BURST_SPRITE_FRAME_BYTES 512
#define FLAK_SHELL_SPRITE_FRAME_BYTES 256

#define MAP_WIDTH_TILES 128
#define MAP_HEIGHT_TILES 256
#define TILE_SIZE_PX 8 
#define SHIP_SPRITE_SIZE_PIXELS 32
#define CROSSHAIR_SPRITE_SIZE_PIXELS 32
#define HI_RES true
#define WIND_GAUGE_X_PX 600
#define WIND_GAUGE_Y_PX 440
#define FLAK_GUN_SPRITE_SIZE_PIXELS 16
#define FLAK_SHELL_SPRITE_SIZE_PIXELS 16
#define FLAK_BURST_SPRITE_SIZE_PIXELS 32

#define WIND_ON false 

#define WIND_CHANGE_CHANCE 300 // of 32767
#define WIND_DIRECTIONS 24

#define THRUST_DIVISOR 32
#define FRICTION_DIVISOR 128
#define WIND_DIVISOR 512
#define FLAK_SHELL_VELOCITY_MULTIPLIER 4

#define MONOPLANE_X_PX 620
#define MONOPLANE_Y_PX 30

#define NUM_FLAK_GUNS 4

#define LOWRES_CENTER_X 132
#define HIRES_CENTER_X 280
#define LOWRES_CENTER_Y 120
#define HIRES_CENTER_Y 240

#define PREDICTOR_LOOKAHEAD_FRAMES 180
#define SHOW_PREDICTOR true

#define NUM_FLAK_SHELLS 20
#define FLAK_FIRE_CHANCE 400 // of 32767
#define FLAK_SHELL_FUSE_FRAMES 65

#define FLAK_BURST_FRAMES_PER_FRAME 4
#define FLAK_BURST_FRAMES 7

#define NUM_CODE_BANKS 1
#define BANK_NUM (*(unsigned char *)0x00)

extern void _BANKRAM01_SIZE__[];

unsigned int tilemap_x_offset_px = MAP_WIDTH_TILES * TILE_SIZE_PX / 2;
unsigned int tilemap_y_offset_px = MAP_HEIGHT_TILES * TILE_SIZE_PX / 2;
//
// THRUST
//
signed long x_comp_for_bearing[NUM_SHIP_BEARINGS] = {
   0,2856,5690,8481,11207,13848,16384,18795,21063,
   23170,25102,26842,28378,29698,30792,31651,32270,32643,
   32767,32643,32270,31651,30792,29698,28378,26842,25102,
   23170,21063,18795,16384,13848,11207,8481,5690,2856,
   0,-2856,-5690,-8481,-11207,-13848,-16384,-18795,-21063,
   -23170,-25102,-26842,-28378,-29698,-30792,-31651,-32270,-32643,
   -32767,-32643,-32270,-31651,-30792,-29698,-28378,-26842,-25102,
   -23170,-21063,-18795,-16384,-13848,-11207,-8481,-5690,-2856 };
signed long y_comp_for_bearing[NUM_SHIP_BEARINGS] = {
   -32767,-32643,-32270,-31651,-30792,-29698,-28378,-26842,-25102,
   -23170,-21063,-18795,-16384,-13848,-11207,-8481,-5690,-2856,
   0,2856,5690,8481,11207,13848,16384,18795,21063,
   23170,25102,26842,28378,29698,30792,31651,32270,32643,
   32767,32643,32270,31651,30792,29698,28378,26842,25102,
   23170,21063,18795,16384,13848,11207,8481,5690,2856,
   0,-2856,-5690,-8481,-11207,-13848,-16384,-18795,-21063,
   -23170,-25102,-26842,-28378,-29698,-30792,-31651,-32270,-32643 };
unsigned char angle_lookup[31][31] = {
  {15,15,15,15,15,15,15,16,16,16,16,17,17,17,17,18,18,18,18,18,19,19,19,19,20,20,20,20,20,20,21},
  {14,15,15,15,15,15,15,16,16,16,16,16,17,17,17,18,18,18,18,19,19,19,19,19,20,20,20,20,20,21,21},
  {14,14,15,15,15,15,15,15,16,16,16,16,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,20,21,21,21},
  {14,14,14,15,15,15,15,15,15,16,16,16,17,17,17,18,18,18,18,19,19,19,20,20,20,20,20,21,21,21,21},
  {14,14,14,14,15,15,15,15,15,16,16,16,16,17,17,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,21},
  {14,14,14,14,14,15,15,15,15,15,16,16,16,17,17,18,18,18,19,19,19,20,20,20,20,21,21,21,21,21,21},
  {14,14,14,14,14,14,15,15,15,15,16,16,16,17,17,18,18,18,19,19,19,20,20,20,21,21,21,21,21,21,21},
  {13,13,14,14,14,14,14,15,15,15,15,16,16,17,17,18,18,18,19,19,20,20,20,21,21,21,21,21,21,22,22},
  {13,13,13,14,14,14,14,14,15,15,15,16,16,16,17,18,18,19,19,19,20,20,21,21,21,21,21,21,22,22,22},
  {13,13,13,13,13,14,14,14,14,15,15,15,16,16,17,18,18,19,19,20,20,21,21,21,21,21,22,22,22,22,22},
  {13,13,13,13,13,13,13,14,14,14,15,15,15,16,17,18,18,19,20,20,21,21,21,21,22,22,22,22,22,22,22},
  {12,13,13,13,13,13,13,13,13,14,14,15,15,16,17,18,18,19,20,21,21,21,22,22,22,22,22,22,22,22,23},
  {12,12,12,12,13,13,13,13,13,13,14,14,15,15,16,18,19,20,21,21,21,22,22,22,22,22,22,23,23,23,23},
  {12,12,12,12,12,12,12,12,13,13,13,13,14,15,16,18,19,21,21,22,22,22,22,23,23,23,23,23,23,23,23},
  {12,12,12,12,12,12,12,12,12,12,12,12,13,13,15,18,21,22,22,23,23,23,23,23,23,23,23,23,23,23,23},
  {12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {11,11,11,11,11,11,11,11,11,11,11,11,10,10,9,6,3,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
  {11,11,11,11,11,11,11,11,10,10,10,10,9,9,7,6,4,3,2,1,1,1,1,0,0,0,0,0,0,0,0},
  {11,11,11,11,10,10,10,10,10,10,9,9,9,8,7,6,4,3,3,2,2,1,1,1,1,1,1,0,0,0,0},
  {11,10,10,10,10,10,10,10,10,9,9,9,8,7,6,6,5,4,3,3,2,2,1,1,1,1,1,1,1,1,0},
  {10,10,10,10,10,10,10,9,9,9,9,8,8,7,6,6,5,4,3,3,3,2,2,2,1,1,1,1,1,1,1},
  {10,10,10,10,10,9,9,9,9,9,8,8,7,7,6,6,5,4,4,3,3,3,2,2,2,2,1,1,1,1,1},
  {10,10,10,9,9,9,9,9,9,8,8,7,7,7,6,6,5,4,4,4,3,3,3,2,2,2,2,2,1,1,1},
  {10,10,9,9,9,9,9,9,8,8,8,7,7,6,6,6,5,5,4,4,3,3,3,3,2,2,2,2,2,1,1},
  {9,9,9,9,9,9,9,8,8,8,7,7,7,6,6,6,5,5,4,4,4,3,3,3,3,2,2,2,2,2,2},
  {9,9,9,9,9,9,8,8,8,8,7,7,7,6,6,6,5,5,4,4,4,3,3,3,3,3,2,2,2,2,2},
  {9,9,9,9,9,8,8,8,8,7,7,7,7,6,6,6,5,5,4,4,4,4,3,3,3,3,3,2,2,2,2},
  {9,9,9,9,8,8,8,8,8,7,7,7,6,6,6,6,5,5,5,4,4,4,3,3,3,3,3,3,2,2,2},
  {9,9,9,8,8,8,8,8,7,7,7,7,6,6,6,6,5,5,5,4,4,4,4,3,3,3,3,3,3,2,2},
  {9,9,8,8,8,8,8,7,7,7,7,7,6,6,6,6,5,5,5,4,4,4,4,4,3,3,3,3,3,3,2},
  {9,8,8,8,8,8,8,7,7,7,7,6,6,6,6,6,5,5,5,5,4,4,4,4,3,3,3,3,3,3,3}
};

//
// VELOCITY
//
signed long ship_vx_fpx = 0;
signed long ship_vy_fpx = 0;
signed long ship_vx_friction = 0;
signed long ship_vy_friction = 0;

//
// POSITION
//
unsigned long ship_x_fpx;
unsigned long ship_y_fpx;
unsigned int ship_x_px;
unsigned int ship_y_px;
unsigned long ship_x_predict_fpx;
unsigned long ship_y_predict_fpx;
unsigned int ship_x_predict_px;
unsigned int ship_y_predict_px;

//
// BEARING AND TURN RATE
//
signed int   bearing_fdegs = 0;
unsigned int bearing_deg = 0;
unsigned int turn_rate_fdegs_pf = 240;

//
// WIND
//
signed char wind_direction = 0;

//
// PUTTING SPRITES on SCREEN 
//
unsigned char bearing_frame = 0;
unsigned long ship_sprite_frame_addr = 0;
unsigned char ship_sprite_flips;
unsigned char needle_sprite_frame;
unsigned long needle_sprite_frame_addr;
unsigned int ship_screen_x_px = 0;
unsigned int ship_screen_y_px = 0;
unsigned long monoplane_sprite_frame_addr;
unsigned char monoplane_frame;
unsigned char flak_frame;
unsigned int crosshair_screen_x_px = 0;
unsigned int crosshair_screen_y_px = 0;
unsigned int screen_center_x_px = 0;
unsigned int screen_center_y_px = 0;

unsigned char joy;
unsigned long game_frame = 0;

typedef struct {
  unsigned char flips;
  unsigned long frame_addr;
} SpriteFrame;

typedef struct {
  unsigned int x_px;
  unsigned int y_px;
  unsigned char bearing;
} FlakGun;

SpriteFrame* sprite_frame;
FlakGun* flak_guns[NUM_FLAK_GUNS];
unsigned int flak_screen_x_px;
unsigned int flak_screen_y_px;

unsigned int hscroll;
unsigned int vscroll;

signed int xoffset;
signed int yoffset;
bool hide_sprite = false;

unsigned int flak_burst_screen_x_px;
unsigned int flak_burst_screen_y_px;
unsigned char flak_burst_frame;
unsigned long flak_burst_frame_addr;

typedef struct {
  unsigned int x_12_4_fpx;
  unsigned int y_12_4_fpx;
  signed int vx_12_4_fpx;
  signed int vy_12_4_fpx;
  bool free;
  unsigned char fuse;
  unsigned char index;
  unsigned long bearing_frame_addr;
  unsigned char flips;
} FlakShell;
FlakShell* flak_shell_pool[NUM_FLAK_SHELLS];
FlakShell* flak_shell;
unsigned int flak_shell_x_px;
unsigned int flak_shell_y_px;
unsigned int flak_shell_screen_x_px;
unsigned int flak_shell_screen_y_px;

typedef struct {
  unsigned long x_px;
  unsigned long y_px;
  bool free;
  unsigned char frame;
  unsigned char index;
  unsigned long exploded_game_frame;
} FlakBurst;

FlakBurst* flak_burst_pool[NUM_FLAK_SHELLS];
FlakBurst* flak_burst;
unsigned int flak_burst_x_px;
unsigned int flak_burst_y_px;
unsigned int flak_burst_screen_x_px;
unsigned int flak_burst_screen_y_px;

unsigned int roll;
clock_t start_time;
clock_t end_time;
unsigned long runtime_seconds;

unsigned char areg;

void load_code_banks(void) {
  int i;
  unsigned char fileName[32];
  unsigned char previousBank = BANK_NUM;
  printf("\n");

  for (i = 1; i <= NUM_CODE_BANKS; i++) {
    sprintf(fileName, "hotair.prg.0%x", i);
    printf("loading %s.....", fileName);

    // Set the Bank we are loading into
    BANK_NUM = i;

    // Load the file into Banked RAM
    cbm_k_setnam(fileName);
    cbm_k_setlfs(0, 8, 0); // Skip the first 2 bytes of the file. They just hold the size in bytes.
    cbm_k_load(0, (unsigned int *) BANK_RAM);
    printf("loaded\n");
  }

  BANK_NUM = previousBank;
}

#pragma code-name (push, "BANKRAM01")
void setup_random(void) {
  // call entropy_get to seed the random number generator
  asm("jsr $FECF");
  asm("STA %v", areg);
  srand(areg);
}

void load_into_vera(char* filename, unsigned long base_addr, char secondary_address) {

  unsigned char m = 2;

  // These 3 functions are basic wrappers for the Kernal Functions

  // You have to first set the name of the file you are working with.
  cbm_k_setnam(filename);

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
}
void vera_setup(void) {

  // petsci upper / gfx

  asm("lda #2");
  asm("jsr $FF62");

#ifdef DEBUG_CONSOLE  
  return;
#endif

  load_into_vera("map0.bin", MAP0_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("sprite0.bin", SHIP_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("map1.bin", MAP1_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("sprite1.bin", NEEDLE_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("circle.bin", CIRCLE_SPRITE_BASE_ADDR, SKIP_2_BYTE_HEADER);
  load_into_vera("monoplane16.bin", MONOPLANE_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  load_into_vera("flak16.bin", FLAK_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  load_into_vera("crosshair32.bin", CROSSHAIR_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  load_into_vera("flakburst32.bin", FLAK_BURST_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  load_into_vera("flakshell16.bin", FLAK_SHELL_SPRITE_BASE_ADDR, NO_2_BYTE_HEADER);
  load_into_vera("palette.bin", PALETTE_BASE_ADDR, NO_2_BYTE_HEADER);

  VERA.display.video = 0b01110001;    // activate layers & sprites
  VERA.display.hscale = HI_RES ? 128 : 64;
  VERA.display.vscale = HI_RES ? 128 : 64;

  VERA.layer0.mapbase = (MAP0_BASE_ADDR >> 9) & 0xFF;  // top eight bits of 17-bit address and 16x16

  VERA.layer0.config = 0b11100000;
  VERA.layer0.tilebase =
    (CHARSET_BASE_ADDR >> 9)  // top six bits of 17-bit address 
    & 0b11111100;             // tile height / width = 8px

  VERA.layer1.config = 0b01100000;  // 128(w)x64(h) 16-color tiles
  VERA.layer1.mapbase = (MAP1_BASE_ADDR >> 9) & 0b11111100;  // top eight bits of 17-bit address and 8x8
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
}
void update_wind(void) {

  // wind_direction = (wind_direction +1) % WIND_DIRECTIONS;
  if (rand() < WIND_CHANGE_CHANCE) {
    wind_direction = wind_direction + ((rand() % 3) - 1);
  }
  if (wind_direction < 0) {
    wind_direction = WIND_DIRECTIONS + wind_direction;
  }
  wind_direction = wind_direction % WIND_DIRECTIONS;
}
void do_mallocs(void) {
  char i;
  for (i = 0; i < NUM_FLAK_GUNS; i++) {
    flak_guns[i] = malloc(sizeof(FlakGun));
  }
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    flak_shell_pool[i] = malloc(sizeof(FlakShell));
  }
  sprite_frame = malloc(sizeof(SpriteFrame));
}
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
void sprite72_frame(SpriteFrame* sf, unsigned long base_addr, unsigned int frame_size_bytes, unsigned char frame) {

  if (frame >= 0 && frame <= 18) {
    sf->flips = 0b00;
    sf->frame_addr = base_addr + (frame_size_bytes * frame);
  }
  else if (frame >= 19 && frame <= 36) {
    sf->flips = 0b10;
    sf->frame_addr = base_addr
      + (frame_size_bytes * (36 - frame));
  }
  else if (frame >= 37 && frame <= 54) {
    sf->flips = 0b11;
    sf->frame_addr = base_addr
      + (frame_size_bytes * ((frame - 36)));
  }
  else if (frame >= 55 && frame <= 71) {
    sf->flips = 0b01;
    sf->frame_addr = base_addr
      + (frame_size_bytes * (72 - frame));
  }

}
void* get_free_object_from_pool(void** pool, unsigned char pool_size, unsigned char free_field_offset) {
  char i;
  for (i = 0; i < pool_size; i++) {
    bool* free_field = (bool*)((char*)pool[i] + free_field_offset);
    if (*free_field == true) {
      return pool[i];
    }
  }
  return NULL;  // No free object found
}

void update_ship_position(void) {
  //
  // THRUST
  //
  if (JOY_UP(joy)) {
    ship_vx_fpx = ship_vx_fpx + x_comp_for_bearing[bearing_frame] / THRUST_DIVISOR;
    ship_vy_fpx = ship_vy_fpx + y_comp_for_bearing[bearing_frame] / THRUST_DIVISOR;
  }

  //
  // FRICTION
  //
  ship_vx_fpx -= ship_vx_fpx / FRICTION_DIVISOR;
  ship_vy_fpx -= ship_vy_fpx / FRICTION_DIVISOR;

  //
  // WIND
  //
  if (WIND_ON) {
    ship_vx_fpx += x_comp_for_bearing[wind_direction * 3] / WIND_DIVISOR;
    ship_vy_fpx += y_comp_for_bearing[wind_direction * 3] / WIND_DIVISOR;
  }

  // 
  // UPDATE VELOCITY
  //
  ship_x_fpx += ship_vx_fpx;
  ship_y_fpx += ship_vy_fpx;

  //
  // DIVIDE BY 256 to go from fpx to px
  //
  ship_x_px = ship_x_fpx >> 16;
  ship_y_px = ship_y_fpx >> 16;

  //
  // FORECAST
  //
  ship_x_predict_fpx = ship_x_fpx + (PREDICTOR_LOOKAHEAD_FRAMES * ship_vx_fpx);
  ship_y_predict_fpx = ship_y_fpx + (PREDICTOR_LOOKAHEAD_FRAMES * ship_vy_fpx);
  ship_x_predict_px = ship_x_predict_fpx >> 16;
  ship_y_predict_px = ship_y_predict_fpx >> 16;

}
void update_ship_bearing(void) {
  if (JOY_LEFT(joy)) {
    bearing_fdegs = bearing_fdegs - turn_rate_fdegs_pf;
    if (bearing_fdegs < 0) {
      bearing_fdegs = 359 * 64 + bearing_fdegs;
    }
  }
  else if (JOY_RIGHT(joy)) {
    bearing_fdegs = bearing_fdegs + turn_rate_fdegs_pf;
    if (bearing_fdegs > 359 * 64) {
      bearing_fdegs = bearing_fdegs - 359 * 64;
    }
  }

  bearing_deg = bearing_fdegs >> 6;
  bearing_frame = (bearing_deg / (DEGREES_PER_FACING)) % NUM_SHIP_BEARINGS;
}
void update_scroll(void) {
  hscroll = ship_x_px - (HI_RES ? HIRES_CENTER_X : LOWRES_CENTER_X);
  vscroll = ship_y_px - (HI_RES ? HIRES_CENTER_Y : LOWRES_CENTER_Y);
}
void vera_scroll(void) {
#ifdef DEBUG_CONSOLE
  return;
#endif  
  VERA.layer0.hscroll = hscroll;
  VERA.layer0.vscroll = vscroll;
}
void update_sprites(void) {
  char i;

#ifdef DEBUG_CONSOLE
  return;
#endif

  VERA.address = SPRITE_ATTR_BASE_ADDR;
  VERA.address_hi = SPRITE_ATTR_BASE_ADDR >> 16;
  VERA.address_hi |= VERA_INC_1;

  sprite72_frame(sprite_frame, SHIP_SPRITE_BASE_ADDR, SHIP_SPRITE_FRAME_BYTES, bearing_frame);

  VERA.data0 = sprite_frame->frame_addr >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | sprite_frame->frame_addr >> 13;
  VERA.data0 = ship_screen_x_px;
  VERA.data0 = ship_screen_x_px >> 8;
  VERA.data0 = ship_screen_y_px;
  VERA.data0 = ship_screen_y_px >> 8;
  VERA.data0 = 0b00001100 | sprite_frame->flips; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000; // 32x32 pixel image

  //
  // WIND INDICATOR NEEDLE
  //
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

  //
  // WIND GAUGE CIRCLE
  //
  VERA.data0 = CIRCLE_SPRITE_BASE_ADDR >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (CIRCLE_SPRITE_BASE_ADDR >> 13);
  VERA.data0 = WIND_GAUGE_X_PX;
  VERA.data0 = WIND_GAUGE_X_PX >> 8;
  VERA.data0 = WIND_GAUGE_Y_PX;
  VERA.data0 = WIND_GAUGE_Y_PX >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = 0b10100000; // 32x32 pixel image

  //
  // MONOPLANE
  //
  monoplane_frame = ((game_frame / 6) % 24);
  sprite24_frame(sprite_frame, MONOPLANE_SPRITE_BASE_ADDR, MONOPLANE_SPRITE_FRAME_BYTES, monoplane_frame);
  VERA.data0 = sprite_frame->frame_addr >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (sprite_frame->frame_addr >> 13);
  VERA.data0 = MONOPLANE_X_PX;
  VERA.data0 = MONOPLANE_X_PX >> 8;
  VERA.data0 = MONOPLANE_Y_PX;
  VERA.data0 = MONOPLANE_Y_PX >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L2 | sprite_frame->flips; // Z-Depth=3, Sprite in front of layer 1
  VERA.data0 = SPRITE_BYTE7_HEIGHT_16 | SPRITE_BYTE7_WIDTH_16;

  //
  // FLAK GUNS
  //
  for (i = 0; i < NUM_FLAK_GUNS; i++) {

    xoffset = (ship_x_predict_px - flak_guns[i]->x_px);
    yoffset = (flak_guns[i]->y_px - ship_y_predict_px);

    if ((abs(xoffset) > screen_center_x_px) || (abs(yoffset) > screen_center_y_px)) {
      flak_guns[i]->bearing = 0;
    }
    else {
      flak_guns[i]->bearing = angle_lookup[(xoffset + screen_center_x_px) / 18][(yoffset + screen_center_y_px) / 16];
    }

    flak_screen_x_px = flak_guns[i]->x_px - hscroll - (FLAK_GUN_SPRITE_SIZE_PIXELS / 2);
    flak_screen_y_px = flak_guns[i]->y_px - vscroll - (FLAK_GUN_SPRITE_SIZE_PIXELS / 2);

    sprite24_frame(sprite_frame, FLAK_SPRITE_BASE_ADDR, FLAK_SPRITE_FRAME_BYTES, flak_guns[i]->bearing);
    VERA.data0 = sprite_frame->frame_addr >> 5;
    VERA.data0 = SPRITE_BYTE1_4BPP | (sprite_frame->frame_addr >> 13);
    VERA.data0 = flak_screen_x_px;
    VERA.data0 = flak_screen_x_px >> 8;
    VERA.data0 = flak_screen_y_px;
    VERA.data0 = flak_screen_y_px >> 8;
    VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L1 | sprite_frame->flips; 
    VERA.data0 = SPRITE_BYTE7_HEIGHT_16 | SPRITE_BYTE7_WIDTH_16;
  }

  //
  // PREDICTOR CROSSHAIR
  //
  crosshair_screen_x_px = ship_x_predict_px - hscroll - (CROSSHAIR_SPRITE_SIZE_PIXELS / 2);
  crosshair_screen_y_px = ship_y_predict_px - vscroll - (CROSSHAIR_SPRITE_SIZE_PIXELS / 2);

  VERA.data0 = CROSSHAIR_SPRITE_BASE_ADDR >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (CROSSHAIR_SPRITE_BASE_ADDR >> 13);
  VERA.data0 = crosshair_screen_x_px;
  VERA.data0 = crosshair_screen_x_px >> 8;
  VERA.data0 = crosshair_screen_y_px;
  VERA.data0 = crosshair_screen_y_px >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L1 & (SHOW_PREDICTOR ? 0b1111 : 0b0000);
  VERA.data0 = SPRITE_BYTE7_HEIGHT_32 | SPRITE_BYTE7_WIDTH_32;

}

void setup_flak_guns(void) {
  
  flak_guns[0]->x_px = (ship_x_fpx >> 16) - 48;
  flak_guns[0]->y_px = (ship_y_fpx >> 16) - 48;

  flak_guns[1]->x_px = (ship_x_fpx >> 16) + 48;
  flak_guns[1]->y_px = (ship_y_fpx >> 16) - 48;

  flak_guns[2]->x_px = (ship_x_fpx >> 16) + 48;
  flak_guns[2]->y_px = (ship_y_fpx >> 16) + 48;

  flak_guns[3]->x_px = (ship_x_fpx >> 16) - 48;
  flak_guns[3]->y_px = (ship_y_fpx >> 16) + 48;

}
void pivot_flak_guns(void) {
  char i;
  for (i = 0; i < NUM_FLAK_GUNS; i++) {

    xoffset = (ship_x_predict_px - flak_guns[i]->x_px);
    yoffset = (flak_guns[i]->y_px - ship_y_predict_px);

    if ((abs(xoffset) > screen_center_x_px) || (abs(yoffset) > screen_center_y_px)) {
      flak_guns[i]->bearing = 0;
    }
    else {
      flak_guns[i]->bearing = angle_lookup[(xoffset + screen_center_x_px) / 18][(yoffset + screen_center_y_px) / 16];
    }
  }
}
void setup_flak_shells(void) {
  char i;
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    flak_shell_pool[i]->free = true;
    flak_shell_pool[i]->index = i;
  }
}
void fire_flak_guns(void) {
 char gun;
  for (gun = 0; gun < NUM_FLAK_GUNS; gun++) {
    roll = rand();
    if (roll < FLAK_FIRE_CHANCE) {
      flak_shell = (FlakShell*)get_free_object_from_pool((void**)flak_shell_pool, NUM_FLAK_SHELLS, (unsigned char)&((FlakShell*)0)->free);
      if (flak_shell != NULL) {
        flak_shell->free = false;
        flak_shell->x_12_4_fpx = flak_guns[gun]->x_px;
        flak_shell->x_12_4_fpx <<= 4;
        flak_shell->y_12_4_fpx = flak_guns[gun]->y_px;
        flak_shell->y_12_4_fpx <<= 4;

        flak_shell->vx_12_4_fpx = (x_comp_for_bearing[flak_guns[gun]->bearing*3] >> 11) * FLAK_SHELL_VELOCITY_MULTIPLIER;
        flak_shell->vy_12_4_fpx = (y_comp_for_bearing[flak_guns[gun]->bearing*3] >> 11) * FLAK_SHELL_VELOCITY_MULTIPLIER;

        sprite24_frame(sprite_frame, FLAK_SHELL_SPRITE_BASE_ADDR, FLAK_SHELL_SPRITE_FRAME_BYTES, flak_guns[gun]->bearing);
        flak_shell->bearing_frame_addr = sprite_frame->frame_addr;
        flak_shell->flips = sprite_frame->flips;
        flak_shell->fuse = FLAK_SHELL_FUSE_FRAMES;
#ifdef DEBUG_CONSOLE
        printf("flak gun %d fired flak shell %d\n", gun, flak_shell->index);
#endif
      }
    }
  }
}
void update_flak_shells(void) {
  char shell;
  for (shell = 0; shell < NUM_FLAK_SHELLS; shell++) {
    if (!flak_shell_pool[shell]->free) {
      flak_shell_pool[shell]->x_12_4_fpx += flak_shell_pool[shell]->vx_12_4_fpx;
      flak_shell_pool[shell]->y_12_4_fpx += flak_shell_pool[shell]->vy_12_4_fpx;

      if (flak_shell_pool[shell]->fuse > 0) {
        flak_shell_pool[shell]->fuse--;
      }
      else {
#ifdef DEBUG_CONSOLE
        printf("flak shell %d fuse expired\n", flak_shells[shell]->index);
#endif  
        flak_shell_pool[shell]->free = true;
        // flak shell explodes
        flak_burst = (FlakBurst*)get_free_object_from_pool((void**)flak_burst_pool, NUM_FLAK_SHELLS, (unsigned char)&((FlakBurst*)0)->free);
        if (flak_burst != NULL) {
          flak_burst->free = false;
          flak_burst->x_px = flak_shell_pool[shell]->x_12_4_fpx >> 4;
          flak_burst->y_px = flak_shell_pool[shell]->y_12_4_fpx >> 4;
#ifdef DEBUG_CONSOLE
          printf("flak shell %d exploded at (%ld, %ld)\n", flak_shells[shell]->index, flak_burst->x_px, flak_burst->y_px);
#endif
          flak_burst->exploded_game_frame = game_frame;
          flak_burst->frame = 0;
        }
        else{
#ifdef DEBUG_CONSOLE
          printf("flak shell %d has no burst available\n", flak_shells[shell]->index);
#endif
        }
      }
#ifdef DEBUG_CONSOLE
      // printf("flakshell %d world_fpx=(%ld, %ld) bearing=%d fuse=%d\n", 
      //   flak_shells[i]->index, flak_shells[i]->x_fpx, flak_shells[i]->y_fpx, flak_shells[i]->bearing, flak_shells[i]->fuse);
#endif
    }
  }
}
void update_flak_shell_sprites(void){
  char shell;
  int flak_shell_xoff_px = hscroll + (FLAK_SHELL_SPRITE_SIZE_PIXELS / 2);
  int flak_shell_yoff_px = vscroll + (FLAK_SHELL_SPRITE_SIZE_PIXELS / 2);

#ifdef DEBUG_CONSOLE
  return;
#endif

  for(shell=0; shell< NUM_FLAK_SHELLS; shell++) {
    flak_shell_x_px = flak_shell_pool[shell]->x_12_4_fpx >> 4;
    flak_shell_y_px = flak_shell_pool[shell]->y_12_4_fpx >> 4;
    flak_shell_screen_x_px = flak_shell_x_px - flak_shell_xoff_px;
    flak_shell_screen_y_px = flak_shell_y_px - flak_shell_yoff_px;

    VERA.data0 = flak_shell_pool[shell]->bearing_frame_addr >> 5;
    VERA.data0 = SPRITE_BYTE1_16BPP | (flak_shell_pool[shell]->bearing_frame_addr >> 13);
    VERA.data0 = flak_shell_screen_x_px;
    VERA.data0 = flak_shell_screen_x_px >> 8;
    VERA.data0 = flak_shell_screen_y_px;
    VERA.data0 = flak_shell_screen_y_px >> 8;
    VERA.data0 = flak_shell_pool[shell]->flips | (flak_shell_pool[shell]->free ? SPRITE_BYTE6_Z_DISABLED : SPRITE_BYTE6_Z_ABOVE_L1);
    VERA.data0 = SPRITE_BYTE7_HEIGHT_16 | SPRITE_BYTE7_WIDTH_16;
  }
}
void setup_flak_bursts(void) {
  char burst;
  for (burst = 0; burst < NUM_FLAK_SHELLS; burst++) {
    flak_burst_pool[burst] = malloc(sizeof(FlakBurst));
    flak_burst_pool[burst]->free = true;
    flak_burst_pool[burst]->index = burst;
  }
}
void update_flak_bursts(void) {
  char i;
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    if (!flak_burst_pool[i]->free) {
      flak_burst->frame = (game_frame - flak_burst_pool[i]->exploded_game_frame) / FLAK_BURST_FRAMES_PER_FRAME;
      if (flak_burst->frame >= FLAK_BURST_FRAMES) {
        flak_burst_pool[i]->free = true;
      }
    }
  }
}
void update_flak_burst_sprites(void) {
  char burst;

  for (burst = 0; burst < NUM_FLAK_SHELLS; burst++) {
    flak_burst_screen_x_px = flak_burst_pool[burst]->x_px  - hscroll - (FLAK_BURST_SPRITE_SIZE_PIXELS / 2);
    flak_burst_screen_y_px = flak_burst_pool[burst]->y_px  - vscroll - (FLAK_BURST_SPRITE_SIZE_PIXELS / 2);
    flak_burst_frame_addr = FLAK_BURST_SPRITE_BASE_ADDR + (flak_burst_pool[burst]->frame * FLAK_BURST_SPRITE_FRAME_BYTES);

#ifdef DEBUG_CONSOLE
    if (! flak_bursts[burst]->free) {
      printf("rendering flak burst %d at (%d, %d)\n", burst, flak_burst_screen_x_px, flak_burst_screen_y_px);
    }
    continue;
#endif

    VERA.data0 = flak_burst_frame_addr >> 5;
    VERA.data0 = SPRITE_BYTE1_4BPP | (flak_burst_frame_addr >> 13);
    VERA.data0 = flak_burst_screen_x_px;
    VERA.data0 = flak_burst_screen_x_px >> 8;
    VERA.data0 = flak_burst_screen_y_px;
    VERA.data0 = flak_burst_screen_y_px >> 8;
    VERA.data0 = (flak_burst_pool[burst]->free ? SPRITE_BYTE6_Z_DISABLED : SPRITE_BYTE6_Z_ABOVE_L1);
    VERA.data0 = SPRITE_BYTE7_HEIGHT_32 | SPRITE_BYTE7_WIDTH_32;
  }
}

#define FIRE_NUM_FG_COLORS 4
#define FIRE_NUM_BG_COLORS 8

unsigned char fire_fgcolors[FIRE_NUM_FG_COLORS] = {0x1, 0x07, 0xA};
unsigned char fire_bgcolors[FIRE_NUM_BG_COLORS] = {0x0, 0x2, 0x2, 0x07, 0x8, 0x8, 0x8, 0x8};

void fire(unsigned char col, unsigned char row, unsigned char size) {
  unsigned char r,c,color;
  unsigned long addr;

  for (r = 0; r < size; r++) {
    for (c = 0; c < size; c++) {
      if (rand() < 4000) {
        addr = 1+ MAP0_BASE_ADDR + (2 * ((row + r) * MAP_WIDTH_TILES + col + c));
        VERA.address = addr;
        VERA.address_hi = addr >> 16;
        VERA.address_hi |= VERA_INC_2;
        color = fire_bgcolors[rand() % FIRE_NUM_BG_COLORS];
        color <<= 4;
        color |= fire_fgcolors[rand() % FIRE_NUM_FG_COLORS];
        VERA.data0 = color ;
      }
    }
  }
}

void outro(void) {
  unsigned long fps = 0;

  end_time = clock();
  runtime_seconds = (end_time - start_time) / CLOCKS_PER_SEC;
  fps = game_frame / runtime_seconds;

  // Reset VERA to text mode
  VERA.display.video = 0b00100001;  // Reset to text mode with only layer 1 active
  VERA.display.hscale = 64;         // Reset scale to 320x240
  VERA.display.vscale = 64;         // Reset scale to 320x240

  // Reset layer 0 to default text mode configuration
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
  VERA.layer1.config = 0b01100000;  // 128x64
  VERA.layer1.mapbase = (0x1b000 >> 9) & 0b11111100;

  printf("\n\nend of game");

  printf("\n\n_bankram01_size__ %u", _BANKRAM01_SIZE__);
  printf("\n\nframes: %lu", game_frame);
  printf("\nruntime: %lu seconds", runtime_seconds);
  printf("\nfps: %lu\n\n", fps);
}

#pragma code-name (pop)

void main(void) {

  bool run = true;

  BANK_NUM = 1;

  start_time = clock();

  ship_x_fpx = (MAP_WIDTH_TILES * TILE_SIZE_PX / 2);
  ship_x_fpx = ship_x_fpx << 16;
  ship_y_fpx = (MAP_HEIGHT_TILES * TILE_SIZE_PX / 2);
  ship_y_fpx = ship_y_fpx << 16;

  load_code_banks();
  setup_random();
  vera_setup();
  joy_install(cx16_std_joy);
  do_mallocs();
  setup_flak_guns();
  setup_flak_shells();
  setup_flak_bursts();

  wind_direction = rand() % 24;
  screen_center_x_px = (HI_RES ? HIRES_CENTER_X : LOWRES_CENTER_X);
  screen_center_y_px = (HI_RES ? HIRES_CENTER_Y : LOWRES_CENTER_Y);
  ship_screen_x_px = screen_center_x_px - (SHIP_SPRITE_SIZE_PIXELS / 2);
  ship_screen_y_px = screen_center_y_px - (SHIP_SPRITE_SIZE_PIXELS / 2);

  while (run) {
    joy = joy_read(0);

    if (JOY_DOWN(joy)) {
      run = false;
    }

    update_wind();
    update_ship_bearing();
    update_ship_position();
    update_scroll();

    pivot_flak_guns();
    update_flak_shells();
    update_flak_bursts();
    fire_flak_guns();

    vera_scroll();
    update_sprites();
    update_flak_shell_sprites();
    update_flak_burst_sprites();


    fire(62,146,5);

    game_frame++;
    wait();
  }

  outro();

}

