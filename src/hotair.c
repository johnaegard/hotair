#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdbool.h>
#include "wait.h"
#include <joystick.h>

#define NUM_SHIP_BEARINGS 32
#define DEGREES_PER_FACING 360/NUM_SHIP_BEARINGS

#define MAP_BASE_ADDR 0x00000
#define MAP_WIDTH_TILES 128
#define MAP_HEIGHT_TILES 256
#define TILE_SIZE_PX 16 
#define TILES_BASE_ADDR 0x10000
#define SHIP_SPRITE_BASE_ADDR 0x14000
#define SHIP_SPRITE_SIZE_PIXELS 32
#define SHIP_SPRITE_FRAME_BYTES 512
#define SPRITE_BASE_ADDR 0x1FC08
#define HI_RES false

void load_into_vera_ignore_header(unsigned char* filename, unsigned long base_addr) {

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

  cbm_k_setlfs(0, 8, 0);

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

unsigned short tilemap_x_offset_px = MAP_WIDTH_TILES * TILE_SIZE_PX / 2;
unsigned short tilemap_y_offset_px = MAP_HEIGHT_TILES * TILE_SIZE_PX / 2;

// position is an unsigned 32-bit in 64ths of a pixel that is >> 6 to translate into px/frame
// velocity component is a signed 16-bit in 64ths of a pixel that is >> 6 to translate into px/frame
// engine thrust in an unsigned 16-bit in 64ths
// angular components are signed chars in 64ths

unsigned int   ship_x_64th_px    = 64 * MAP_WIDTH_TILES * TILE_SIZE_PX / 2;
unsigned int   ship_y_64th_px    = 64 * MAP_HEIGHT_TILES * TILE_SIZE_PX / 2;
unsigned short ship_x_px;
unsigned short ship_y_px;

signed short   ship_vx_64th_px   = 0; 
signed short   ship_vy_64th_px   = 0; 
unsigned short thrust_64th_px    = 1; 

unsigned short ship_sprite_x_px = 0;
unsigned short ship_sprite_y_px = 0;

// bearing is a signed 16-bit in in 64ths  (signed only to detect wraparounds)
signed short   bearing_64th_degs      = 0;
unsigned short bearing_deg            = 0;
unsigned char  bearing_frame          = 0;
unsigned long  ship_sprite_frame_addr = 0;

signed char x_comp_for_bearing_frame[32] = {0,12,24,36,45,53,59,63,64,63,59,53,45,36,24,12,
                                             0,-12,-24,-36,-45,-53,-59,-63,-64,-63,-59,-53,-45,-36,-24,-12};
signed char y_comp_for_bearing_frame[32] = {-64,-63,-59,-53,-45,-36,-24,-12,0,12,24,36,45,53,59,63,64,
                                             63,59,53,45,36,24,12,0,-12,-24,-36,-45,-53,-59,-63};

// turn rate is an unsigned 8-bit number in 64ths
unsigned short turn_rate_64th_degs_per_frame = 380;

unsigned char joy;

void vera_setup() {
  load_into_vera_ignore_header("map0.bin", MAP_BASE_ADDR);
  load_into_vera_ignore_header("tiles.bin", TILES_BASE_ADDR);
  load_into_vera_ignore_header("sprite0.bin", SHIP_SPRITE_BASE_ADDR);

  VERA.display.video |= 0b01100000;    // activate layer 1 & sprites
  VERA.display.hscale = HI_RES ? 128 : 64;
  VERA.display.vscale = HI_RES ? 128 : 64;

  VERA.layer1.config = 0b11100010;
  VERA.layer1.mapbase = (MAP_BASE_ADDR >> 9) & 0xFF;  // top eight bits of 17-bit address

  VERA.layer1.tilebase =
    (TILES_BASE_ADDR >> 9) // top six bits of 17-bit address 
    | 0b11;                 // tile height / width = 16px
}

void main() {

  vera_setup();
  joy_install(cx16_std_joy);

  ship_sprite_x_px = HI_RES ? 320 : 160;
  ship_sprite_y_px = HI_RES ? 240 : 120;
  ship_sprite_x_px -= SHIP_SPRITE_SIZE_PIXELS / 2;
  ship_sprite_y_px -= SHIP_SPRITE_SIZE_PIXELS / 2;

  while (true) {

    joy = joy_read(0);

    if (JOY_LEFT(joy)) {
      bearing_64th_degs = bearing_64th_degs - turn_rate_64th_degs_per_frame;
      if (bearing_64th_degs < 0) {
        bearing_64th_degs = 360 * 64 + bearing_64th_degs;
      }
    } else if (JOY_RIGHT(joy)) {
      bearing_64th_degs = bearing_64th_degs + turn_rate_64th_degs_per_frame;
      if (bearing_64th_degs > 359 * 64) {
        bearing_64th_degs = bearing_64th_degs - 360*64;
      }
    }

    bearing_deg = bearing_64th_degs >> 6;
    bearing_frame = (bearing_deg / (DEGREES_PER_FACING)) % NUM_SHIP_BEARINGS;
    ship_sprite_frame_addr = SHIP_SPRITE_BASE_ADDR + (SHIP_SPRITE_FRAME_BYTES * bearing_frame);

    if(JOY_UP(joy)) {
      ship_vx_64th_px = ship_vx_64th_px + x_comp_for_bearing_frame[bearing_frame];
      ship_vy_64th_px = ship_vy_64th_px + y_comp_for_bearing_frame[bearing_frame];
    }

    // ship_vx_64th_px = 128;
    // ship_vy_64th_px = 32;

    ship_x_64th_px += ship_vx_64th_px;
    ship_y_64th_px += ship_vy_64th_px;

    ship_x_px = ship_x_64th_px >> 6;
    ship_y_px = ship_y_64th_px >> 6;

    VERA.layer1.hscroll = ship_x_px - (HI_RES ? 320 : 160);
    VERA.layer1.vscroll = ship_y_px - (HI_RES ? 240 : 120);

    // Point to Sprite 1
    VERA.address = SPRITE_BASE_ADDR;
    VERA.address_hi = SPRITE_BASE_ADDR >> 16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    // Configure Sprite 1
    // Graphic address bits 12:5
    VERA.data0 = ship_sprite_frame_addr >> 5;
    // 16 color mode, and graphic address bits 16:13
    VERA.data0 = 0b10001111 & ship_sprite_frame_addr >> 13;
    VERA.data0 = ship_sprite_x_px; 
    VERA.data0 = ship_sprite_x_px >> 8;
    VERA.data0 = ship_sprite_y_px; 
    VERA.data0 = ship_sprite_y_px >> 8;
    VERA.data0 = 0b00001100; // Z-Depth=3, Sprite in front of layer 1
    VERA.data0 = 0b10100000; // 32x32 pixel image

    wait();

  }

}