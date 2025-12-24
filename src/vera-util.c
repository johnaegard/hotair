#include <cx16.h>
#include <cbm.h>
#include <stdio.h>
#include <stdlib.h>
#include "vera-constants.h"
#include "vera-util.h"

// External variables and definitions needed from fire.c
extern unsigned char file_error_num;

// VERA LOAD ADDRESSES (from fire.c)
#define MAP0_ADDR 0x0000
#define MAP1_ADDR 0x2000
#define NEEDLE_SPRITE_BITMAP_ADDR 0x6000
#define CIRCLE_SPRITE_BITMAP_ADDR 0x6E00
#define FACE_SPRITE_BITMAP_ADDR 0x7000
#define TILESET_ADDR 0x1F000

#define SKIP_2_BYTE_HEADER 0

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
