#include <cx16.h>
#include <cbm.h>
#include <string.h>
#include <stdbool.h>
#include <joystick.h>

#include "wait.h"

#define NUM_SHIP_BEARINGS 72
#define DEGREES_PER_FACING 360/NUM_SHIP_BEARINGS

#define MAP0_BASE_ADDR 0x00000
#define SHIP_SPRITE_BASE_ADDR 0x10000
#define MAP1_BASE_ADDR 0x12800
#define NEEDLE_SPRITE_BASE_ADDR 0x13800
#define CHARSET_BASE_ADDR 0x1F000
#define SPRITE_ATTR_BASE_ADDR 0x1FC08

#define MAP_WIDTH_TILES 128
#define MAP_HEIGHT_TILES 256
#define TILE_SIZE_PX 16 
#define SHIP_SPRITE_SIZE_PIXELS 32
#define SHIP_SPRITE_FRAME_BYTES 512
#define HI_RES false
#define NEEDLE_SPRITE_X_PX 280
#define NEEDLE_SPRITE_Y_PX 200

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
void vera_setup() {

  // petsci upper / gfx

  asm("lda #2");
  asm("jsr $FF62");

  load_into_vera_ignore_header("map0.bin", MAP0_BASE_ADDR);
  load_into_vera_ignore_header("sprite0.bin", SHIP_SPRITE_BASE_ADDR);
  load_into_vera_ignore_header("map1.bin", MAP1_BASE_ADDR);
  load_into_vera_ignore_header("sprite1.bin", NEEDLE_SPRITE_BASE_ADDR);

  VERA.display.video  = 0b01110001;    // activate layers & sprites
  VERA.display.hscale = HI_RES ? 128 : 64;
  VERA.display.vscale = HI_RES ? 128 : 64;

  VERA.layer0.mapbase = (MAP0_BASE_ADDR >> 9) & 0xFF;  // top eight bits of 17-bit address and 16x16

  VERA.layer0.config = 0b11100000;
  VERA.layer0.tilebase =
     (CHARSET_BASE_ADDR >> 9)  // top six bits of 17-bit address 
     & 0b11111100;             // tile height / width = 8px

  VERA.layer1.config  = 0b00010000;  // 32x64 16-color tiles
  VERA.layer1.mapbase = (MAP1_BASE_ADDR >> 9) & 0b11111100;  // top eight bits of 17-bit address and 8x8
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
}

unsigned int tilemap_x_offset_px = MAP_WIDTH_TILES * TILE_SIZE_PX / 2;
unsigned int tilemap_y_offset_px = MAP_HEIGHT_TILES * TILE_SIZE_PX / 2;

// 
// THRUST
//
signed char thrust_divisor = 48;

signed int x_thrust_for_bearing_frame[NUM_SHIP_BEARINGS] = {
   0,2856,5690,8481,11207,13848,16384,18795,21063,
   23170,25102,26842,28378,29698,30792,31651,32270,32643,
   32767,32643,32270,31651,30792,29698,28378,26842,25102,
   23170,21063,18795,16384,13848,11207,8481,5690,2856,
   0,-2856,-5690,-8481,-11207,-13848,-16384,-18795,-21063,
   -23170,-25102,-26842,-28378,-29698,-30792,-31651,-32270,-32643,
   -32767,-32643,-32270,-31651,-30792,-29698,-28378,-26842,-25102,
   -23170,-21063,-18795,-16384,-13848,-11207,-8481,-5690,-2856};

signed int y_thrust_for_bearing_frame[NUM_SHIP_BEARINGS] = {
   -32767,-32643,-32270,-31651,-30792,-29698,-28378,-26842,-25102,
   -23170,-21063,-18795,-16384,-13848,-11207,-8481,-5690,-2856,
   0,2856,5690,8481,11207,13848,16384,18795,21063,
   23170,25102,26842,28378,29698,30792,31651,32270,32643,
   32767,32643,32270,31651,30792,29698,28378,26842,25102,
   23170,21063,18795,16384,13848,11207,8481,5690,2856,
   0,-2856,-5690,-8481,-11207,-13848,-16384,-18795,-21063,
   -23170,-25102,-26842,-28378,-29698,-30792,-31651,-32270,-32643};



//
// VELOCITY
//
signed long ship_vx_fpx = 0; 
signed long ship_vy_fpx = 0; 
signed char friction_divisor = 80;
signed long ship_vx_friction = 0;
signed long ship_vy_friction = 0;

//
// POSITION
//
unsigned long ship_x_fpx = 32767 * MAP_WIDTH_TILES * TILE_SIZE_PX / 2;
unsigned long ship_y_fpx = 32767 * MAP_HEIGHT_TILES * TILE_SIZE_PX / 2;
unsigned int  ship_x_px;
unsigned int  ship_y_px;

//
// BEARING AND TURN RATE
//
signed int   bearing_fdegs      = 0;
unsigned int bearing_deg        = 0;
unsigned int turn_rate_fdegs_pf = 80;

//
// SPRITE FRAME
//
unsigned char bearing_frame          = 0;
unsigned long ship_sprite_frame_addr = 0;
unsigned char ship_sprite_flips;
unsigned char wind_indicator_frame ;

unsigned int ship_screen_x_px = 0;
unsigned int ship_screen_y_px = 0;

unsigned char joy;

void main() {

  vera_setup();
  joy_install(cx16_std_joy);

  ship_screen_x_px = HI_RES ? 240 : 120;
  ship_screen_y_px = HI_RES ? 240 : 120;
  ship_screen_x_px -= SHIP_SPRITE_SIZE_PIXELS / 2;
  ship_screen_y_px -= SHIP_SPRITE_SIZE_PIXELS / 2;

  while (true) {

    joy = joy_read(0);

    if (JOY_LEFT(joy)) {
      bearing_fdegs = bearing_fdegs - turn_rate_fdegs_pf;
      if (bearing_fdegs < 0) {
        bearing_fdegs = 359 * 64 + bearing_fdegs;
      }
    } else if (JOY_RIGHT(joy)) {
      bearing_fdegs = bearing_fdegs + turn_rate_fdegs_pf;
      if (bearing_fdegs > 359 * 64) {
        bearing_fdegs = bearing_fdegs - 359*64;
      }
    }

    bearing_deg   = bearing_fdegs >> 6;
    bearing_frame = (bearing_deg / (DEGREES_PER_FACING)) % NUM_SHIP_BEARINGS;

    if(JOY_UP(joy)) {
      ship_vx_fpx = ship_vx_fpx + x_thrust_for_bearing_frame[bearing_frame] / thrust_divisor;
      ship_vy_fpx = ship_vy_fpx + y_thrust_for_bearing_frame[bearing_frame] / thrust_divisor;
    }

    ship_vx_fpx -= ship_vx_fpx / friction_divisor;
    ship_vy_fpx -= ship_vy_fpx / friction_divisor;

    ship_x_fpx += ship_vx_fpx;
    ship_y_fpx += ship_vy_fpx;

    ship_x_px = ship_x_fpx >> 16;
    ship_y_px = ship_y_fpx >> 16;

    VERA.layer0.hscroll = ship_x_px - (HI_RES ? 240 : 120);
    VERA.layer0.vscroll = ship_y_px - (HI_RES ? 240 : 120);

    // Point to Sprite 1
    VERA.address = SPRITE_ATTR_BASE_ADDR;
    VERA.address_hi = SPRITE_ATTR_BASE_ADDR >> 16;
    // Set the Increment Mode, turn on bit 4
    VERA.address_hi |= 0b10000;

    if (bearing_frame >= 0 && bearing_frame <= 18) {
      ship_sprite_flips = 0b00;
      ship_sprite_frame_addr = SHIP_SPRITE_BASE_ADDR + (SHIP_SPRITE_FRAME_BYTES * bearing_frame);
    }
    else if (bearing_frame>=19 && bearing_frame <=36) {
      ship_sprite_flips = 0b10;
      ship_sprite_frame_addr = SHIP_SPRITE_BASE_ADDR 
        + (SHIP_SPRITE_FRAME_BYTES * (18 - (bearing_frame -18)));
    }
    else if (bearing_frame>=37 && bearing_frame <=54) {
      ship_sprite_flips = 0b11;
      ship_sprite_frame_addr = SHIP_SPRITE_BASE_ADDR 
        + (SHIP_SPRITE_FRAME_BYTES * ((bearing_frame -36)));
    }
    else if (bearing_frame>=55 && bearing_frame <=71) {
      ship_sprite_flips = 0b01;
      ship_sprite_frame_addr = SHIP_SPRITE_BASE_ADDR 
        + (SHIP_SPRITE_FRAME_BYTES * (18 - (bearing_frame -54)));
    }

    // Configure Sprite 1
    // Graphic address bits 12:5
    VERA.data0 = ship_sprite_frame_addr >> 5;
    // 16 color mode, and graphic address bits 16:13
    VERA.data0 = 0b10001111 & ship_sprite_frame_addr >> 13;
    VERA.data0 = ship_screen_x_px; 
    VERA.data0 = ship_screen_x_px >> 8;
    VERA.data0 = ship_screen_y_px; 
    VERA.data0 = ship_screen_y_px >> 8;
    VERA.data0 = 0b00001100 | ship_sprite_flips; // Z-Depth=3, Sprite in front of layer 1
    VERA.data0 = 0b10100000; // 32x32 pixel image

    wind_indicator_frame = bearing_frame % 7;
    VERA.data0 = NEEDLE_SPRITE_BASE_ADDR + wind_indicator_frame >> 5;
    // 16 color mode, and graphic address bits 16:13
    VERA.data0 = 0b10001111 & NEEDLE_SPRITE_BASE_ADDR >> 13;
    VERA.data0 = NEEDLE_SPRITE_X_PX; 
    VERA.data0 = NEEDLE_SPRITE_X_PX >> 8;
    VERA.data0 = NEEDLE_SPRITE_Y_PX; 
    VERA.data0 = NEEDLE_SPRITE_Y_PX >> 8;
    VERA.data0 = 0b00001100 | ship_sprite_flips; // Z-Depth=3, Sprite in front of layer 1
    VERA.data0 = 0b10100000; // 32x32 pixel image




    wait();
  }

}