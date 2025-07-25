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

#define WIND_ON false 

#define WIND_CHANGE_CHANCE 300 // of 32767
#define WIND_DIRECTIONS 24

#define THRUST_DIVISOR 32
#define FRICTION_DIVISOR 128
#define WIND_DIVISOR 512
#define FLAK_SHELL_VELOCITY_MULTIPLIER 12

#define MONOPLANE_X_PX 620
#define MONOPLANE_Y_PX 30

#define NUM_FLAK_GUNS 4

#define LOWRES_CENTER_X 132
#define HIRES_CENTER_X 280
#define LOWRES_CENTER_Y 120
#define HIRES_CENTER_Y 240

#define PREDICTOR_LOOKAHEAD_FRAMES 180
#define SHOW_PREDICTOR true

#define NUM_FLAK_SHELLS 8
#define FLAK_FIRE_CHANCE 100 // of 32767

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
unsigned char i;
unsigned int flak_screen_x_px;
unsigned int flak_screen_y_px;

unsigned int hscroll;
unsigned int vscroll;

signed int xoffset;
signed int yoffset;

unsigned int flak_burst_screen_x_px;
unsigned int flak_burst_screen_y_px;
unsigned char flak_burst_frame;
unsigned long flak_burst_frame_addr;


unsigned int flak_shell_x_px;
unsigned int flak_shell_y_px;
unsigned int flak_shell_screen_x_px;
unsigned int flak_shell_screen_y_px;

bool hide_sprite = false;

typedef struct {
  unsigned long x_fpx;
  unsigned long y_fpx;
  unsigned char bearing;
  signed long vx_fpx;
  signed long vy_fpx;
  bool free;
  unsigned char fuse;
  unsigned char index;
} FlakShell;
FlakShell* flak_shells[NUM_FLAK_SHELLS];
FlakShell* flak_shell;
unsigned int roll;

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
  for (i = 0; i < NUM_FLAK_GUNS; i++) {
    flak_guns[i] = malloc(sizeof(FlakGun));
  }
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    flak_shells[i] = malloc(sizeof(FlakShell));
  }
  sprite_frame = malloc(sizeof(SpriteFrame));
}
void setup_flak_guns(void) {
  flak_guns[0]->x_px = (ship_x_fpx >> 16) - 56;
  flak_guns[0]->y_px = (ship_y_fpx >> 16) - 56;

  flak_guns[1]->x_px = (ship_x_fpx >> 16) + 40;
  flak_guns[1]->y_px = (ship_y_fpx >> 16) - 56;

  flak_guns[2]->x_px = (ship_x_fpx >> 16) + 40;
  flak_guns[2]->y_px = (ship_y_fpx >> 16) + 40;

  flak_guns[3]->x_px = (ship_x_fpx >> 16) - 56;
  flak_guns[3]->y_px = (ship_y_fpx >> 16) + 40;


}
void setup_flak_shells(void) {
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    flak_shells[i]->free = true;
    flak_shells[i]->index = i;
  }
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
void pivot_flak_guns(void) {
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
void update_sprites(void) {
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

    flak_screen_x_px = flak_guns[i]->x_px - hscroll;
    flak_screen_y_px = flak_guns[i]->y_px - vscroll;

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

  //
  // FLAK BURST
  //
  flak_burst_screen_x_px = 100;
  flak_burst_screen_y_px = 100;

  flak_burst_frame = (game_frame / 2) % 7;
  flak_burst_frame_addr = FLAK_BURST_SPRITE_BASE_ADDR + (flak_burst_frame * FLAK_BURST_SPRITE_FRAME_BYTES);

  VERA.data0 = flak_burst_frame_addr >> 5;
  VERA.data0 = SPRITE_BYTE1_4BPP | (flak_burst_frame_addr >> 13);
  VERA.data0 = flak_burst_screen_x_px;
  VERA.data0 = flak_burst_screen_x_px >> 8;
  VERA.data0 = flak_burst_screen_y_px;
  VERA.data0 = flak_burst_screen_x_px >> 8;
  VERA.data0 = SPRITE_BYTE6_Z_ABOVE_L1;
  VERA.data0 = SPRITE_BYTE7_HEIGHT_32 | SPRITE_BYTE7_WIDTH_32;

}

//
// FLAK
// 
FlakShell* get_free_flak_shell(void) {
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    if (flak_shells[i]->free == true) {
      return flak_shells[i];
    }
  }
  return NULL;  // No free shell found
}

void fire_flak_guns(void) {
  for (i = 0; i < NUM_FLAK_GUNS; i++) {
    roll = rand();
    if (roll < FLAK_FIRE_CHANCE) {
      flak_shell = get_free_flak_shell();
      if (flak_shell != NULL) {
        flak_shell->free = false;
        flak_shell->x_fpx = flak_guns[i]->x_px;
        flak_shell->x_fpx <<= 16;
        flak_shell->y_fpx = flak_guns[i]->y_px;
        flak_shell->y_fpx <<= 16;
        flak_shell->bearing = flak_guns[i]->bearing;
        flak_shell->vx_fpx = x_comp_for_bearing[flak_shell->bearing*3] * FLAK_SHELL_VELOCITY_MULTIPLIER;
        flak_shell->vy_fpx = y_comp_for_bearing[flak_shell->bearing*3] * FLAK_SHELL_VELOCITY_MULTIPLIER;
        flak_shell->fuse = 180;
      
#ifdef DEBUG_CONSOLE
        printf("Flak Gun %d fired flak shell %d with Roll: %d\n", i, flak_shell->index, roll);
#endif
      }
    }
  }
}

void update_flak_shells(void) {
  for (i = 0; i < NUM_FLAK_SHELLS; i++) {
    if (!flak_shells[i]->free) {
      flak_shells[i]->x_fpx += flak_shells[i]->vx_fpx;
      flak_shells[i]->y_fpx += flak_shells[i]->vy_fpx;

      if (flak_shells[i]->fuse > 0) {
        flak_shells[i]->fuse--;
      }
      else {
        flak_shells[i]->free = true; // Mark shell as free when fuse expires
      }
#ifdef DEBUG_CONSOLE
      printf("flakshell %d world_fpx=(%ld, %ld) bearing=%d fuse=%d\n", 
        flak_shells[i]->index, flak_shells[i]->x_fpx, flak_shells[i]->y_fpx, flak_shells[i]->bearing, flak_shells[i]->fuse);
#endif
    }
  }
}

void update_flak_shell_sprites(void){

  for(i=0;i< NUM_FLAK_SHELLS; i++) {
    flak_shell_x_px = flak_shells[i]->x_fpx >> 16;
    flak_shell_y_px = flak_shells[i]->y_fpx >> 16;
    flak_shell_screen_x_px = flak_shell_x_px - hscroll;
    flak_shell_screen_y_px = flak_shell_y_px - vscroll;
#ifdef DEBUG_CONSOLE
    if (! flak_shells[i]->free) {
      printf("  velocity=(%ld,%ld) world=(%d, %d) screen=(%d,%d) bearing=%d\n", 
        flak_shells[i]->vx_fpx, flak_shells[i]->vy_fpx, flak_shell_x_px, flak_shell_y_px, flak_shell_screen_x_px, flak_shell_screen_y_px, flak_shells[i]->bearing);
    }
    continue;
#endif
    sprite24_frame(sprite_frame, FLAK_SHELL_SPRITE_BASE_ADDR, FLAK_SHELL_SPRITE_FRAME_BYTES, flak_shells[i]->bearing);
    VERA.data0 = sprite_frame->frame_addr >> 5;
    VERA.data0 = SPRITE_BYTE1_16BPP | (sprite_frame->frame_addr >> 13);
    VERA.data0 = flak_shell_screen_x_px;
    VERA.data0 = flak_shell_screen_x_px >> 8;
    VERA.data0 = flak_shell_screen_y_px;
    VERA.data0 = flak_shell_screen_y_px >> 8;
    VERA.data0 = sprite_frame->flips | (flak_shells[i]->free ? SPRITE_BYTE6_Z_DISABLED : SPRITE_BYTE6_Z_ABOVE_L2);
    VERA.data0 = SPRITE_BYTE7_HEIGHT_16 | SPRITE_BYTE7_WIDTH_16;
  }
}



void main(void) {
  ship_x_fpx = (MAP_WIDTH_TILES * TILE_SIZE_PX / 2);
  ship_x_fpx = ship_x_fpx << 16;
  ship_y_fpx = (MAP_HEIGHT_TILES * TILE_SIZE_PX / 2);
  ship_y_fpx = ship_y_fpx << 16;

  srand(time(NULL));
  vera_setup();
  joy_install(cx16_std_joy);
  do_mallocs();
  setup_flak_guns();
  setup_flak_shells();

  wind_direction = rand() % 24;
  screen_center_x_px = (HI_RES ? HIRES_CENTER_X : LOWRES_CENTER_X);
  screen_center_y_px = (HI_RES ? HIRES_CENTER_Y : LOWRES_CENTER_Y);
  ship_screen_x_px = screen_center_x_px - (SHIP_SPRITE_SIZE_PIXELS / 2);
  ship_screen_y_px = screen_center_y_px - (SHIP_SPRITE_SIZE_PIXELS / 2);

  while (true) {
    update_wind();
    joy = joy_read(0);
    update_ship_bearing();
    update_ship_position();
    update_scroll();
    vera_scroll();
    pivot_flak_guns();
    update_flak_shells();
    fire_flak_guns();
    update_sprites();
    update_flak_shell_sprites();
    game_frame++;
    wait();
  }

}