#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdbool.h>
#include "wait.h"
#include <joystick.h>

#define NUM_SHIP_BEARINGS 32
#define DEGREES_PER_FACING 360/NUM_SHIP_BEARINGS

#define MAP_BASE_ADDR 0x00000
#define TILES_BASE_ADDR 0x10000
#define SHIP_SPRITE_BASE_ADDR 0x14000
#define SHIP_SPRITE_SIZE_PIXELS 32
#define SHIP_SPRITE_FRAME_BYTES 512
#define SPRITE_BASE_ADDR 0x1FC08

// position is an unsigned 16-bit in 128ths that is bit-shifted 7x to translate into px/frame
// velocity compent is a signed 16-bit int with 128ths that is bit-shifted 7x to translate into px/frame
// engine thrust in an unsigned char in 128ths
// angular components are signed chars in 128ths

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

unsigned long ship_sprite_frame_addr  = 0;
unsigned short x = 160;
unsigned short ship_sprite_x_px = 0;
unsigned short y = 100;
unsigned short ship_sprite_y_px = 0;

// bearing is a signed 16-bit in in 64ths  (signed only to detect wraparounds)
signed short bearing_64th_degs = 0;
unsigned short bearing_deg      = 0;
unsigned char bearing_frame     = 0;

// turn rate is an unsigned 8-bit number in 64ths
unsigned short turn_rate_64th_degs_per_frame = 180;

unsigned char joy;

void main() {

  // Our default Tile and Map Base addresses

  load_into_vera_ignore_header("map0.bin", MAP_BASE_ADDR);
  load_into_vera_ignore_header("tiles.bin", TILES_BASE_ADDR);
  load_into_vera_ignore_header("sprite0.bin", SHIP_SPRITE_BASE_ADDR);

  VERA.display.video |= 0b01100000;    // activate layer 1 & sprites
  VERA.display.hscale = 64;
  VERA.display.vscale = 64;

  VERA.layer1.config = 0b11100010;
  VERA.layer1.mapbase = (MAP_BASE_ADDR >> 9) & 0xFF;  // top eight bits of 17-bit address

  VERA.layer1.tilebase =
    (TILES_BASE_ADDR >> 9) // top six bits of 17-bit address 
    | 0b11;                 // tile height / width = 16px

  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;

  joy_install(cx16_std_joy);

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

    // adjust X/Y to put the sprite at the center of the screen
    ship_sprite_x_px = x - SHIP_SPRITE_SIZE_PIXELS / 2; 
    ship_sprite_y_px = y - SHIP_SPRITE_SIZE_PIXELS / 2; 

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