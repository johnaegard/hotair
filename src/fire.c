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

#define HI_RES true

unsigned char joy;
unsigned long game_frame = 0;
clock_t start_time;
clock_t end_time;
unsigned long runtime_seconds;
unsigned char areg;

#define BANK_NUM (*(unsigned char *)0x00)
#define ARRAY_2D_ADDRESS 0xA000
char (*fire_data)[64] = (char (*)[64])ARRAY_2D_ADDRESS;

void setup_random(void) {
  // call entropy_get to seed the random number generator
  asm("jsr $FECF");
  asm("STA %v", areg);
  srand(areg);
}
void load_into_vera(char* filename, unsigned long base_addr, char secondary_address) {

#define SKIP_2_BYTE_HEADER 0
#define USE_2_BYTE_HEADER 1
#define NO_2_BYTE_HEADER 2

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

  // VERA.layer0.config = 0b11100000;
  VERA.layer0.config = LAYER_MAP_HEIGHT_256 | LAYER_MAP_WIDTH_128 | LAYER_T256C_OFF | LAYER_BITMAP_OFF | LAYER_BPP_1;
  VERA.layer0.tilebase =
    (CHARSET_BASE_ADDR >> 9)  // top six bits of 17-bit address 
    & 0b11111100;             // tile height / width = 8px

  VERA.layer1.config = 0b01100000;  // 128(w)x64(h) 16-color tiles
  VERA.layer1.mapbase = (MAP1_BASE_ADDR >> 9) & 0b11111100;  // top eight bits of 17-bit address and 8x8
  VERA.layer1.hscroll = 0;
  VERA.layer1.vscroll = 0;
}
void outro(void) {
  unsigned long fps = 0;

  end_time = clock();
  runtime_seconds = 1+ ((end_time - start_time) / CLOCKS_PER_SEC);
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

  printf("\n\nframes: %lu", game_frame);
  printf("\nruntime: %lu seconds", runtime_seconds);
  printf("\nfps: %lu\n\n", fps);
}

unsigned char x, y;
void fire_setup(void) {
  for (y = 0; y < 64; y++) {
    for (x = 0; x < 64; x++) {
      fire_data[y][x] = (rand() < 3000) ? 1 : 0;
    }
  }
}

void main(void) {

  bool run = true;

  setup_random();
  vera_setup();
  joy_install(cx16_std_joy);
  fire_setup();

  start_time = clock();

  while (run) {
    joy = joy_read(0);

    if (JOY_DOWN(joy)) {
      run = false;
    }
    game_frame++;
    wait(); 
  }

  outro();

}

