#include <cx16.h>
#include <cbm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../vera-util.h"

#define TILESET_VERA_ADDR 0x1F000
#define TILEMAP_VERA_ADDR 0x0000UL
#define TILESET_FILE "petscii.bin"

void load_tileset_to_vera(void)
{
  load_into_vera(TILESET_FILE, TILESET_VERA_ADDR, SKIP_2_BYTE_HEADER);
}

void setup_tilemap(void)
{
  unsigned int tile_index = 0;
  unsigned int x, y;

  VERA.address = TILEMAP_VERA_ADDR & 0xFFFF;
  VERA.address_hi = (TILEMAP_VERA_ADDR >> 16) | VERA_INC_1;

  for (y = 0; y < 32; y++){
    VERA.data0 = 0x00;
    VERA.data0 = 0x00;
    for (x = 0; x < 64; x++){
      VERA.data0 = 0x00;
      VERA.data0 = 0x00;
    }
  }

  VERA.address = TILEMAP_VERA_ADDR & 0xFFFF;
  VERA.address_hi = (TILEMAP_VERA_ADDR >> 16) | VERA_INC_1;

  for (y = 0; y < 16; y++){
    if (y < 10) {
      VERA.data0 = 0x30 + y;
    }
    else{
      VERA.data0 = y - 9;
    }
    VERA.data0 = 0x01;
    VERA.data0 = 0x00;
    VERA.data0 = 0x00;
  }

  // Create a 16x16 matrix of tiles (0-255)
  for (y = 0; y < 16; y++){
    VERA.address = TILEMAP_VERA_ADDR + (128 * (y + 2));

    if (y < 10) {
      VERA.data0 = 0x30 + y;
    }
    else{
      VERA.data0 = y - 9;
    }
    VERA.data0 = 0x01;
    VERA.data0 = 0x00;
    VERA.data0 = 0x00;

    for (x = 0; x < 16; x++)
    {
      if (tile_index > 255)
      {
        VERA.data0 = 0;
        VERA.data0 = 0x00;
        VERA.data0 = 0;
        VERA.data0 = 0x00;
      }
      else
      {
        VERA.data0 = tile_index;
        VERA.data0 = 0x01;
        VERA.data0 = 0;
        VERA.data0 = 0x00;
        tile_index++;
      }
    }
    // Skip the rest of the tilemap row (64 tiles per row)
    for (x = 16; x < 64; x++)
    {
      VERA.data0 = 0;
      VERA.data0 = 0;
    }
  }
}

void setup_vera(void)
{
  VERA.display.video =
      SPRITES_DISABLED |
      LAYER1_DISABLED |
      LAYER0_ENABLED |
      VGA_ENABLED;

  VERA.display.hscale = DC_HSCALE_320;
  VERA.display.vscale = DC_VSCALE_240;

  VERA.layer0.mapbase = (TILEMAP_VERA_ADDR >> 9);

  VERA.layer0.config =
      LAYER_MAP_HEIGHT_32 |
      LAYER_MAP_WIDTH_64 |
      LAYER_T256C_OFF |
      LAYER_BITMAP_OFF |
      LAYER_BPP_1;

  VERA.layer0.tilebase =
      (TILESET_VERA_ADDR >> 9 & TILE_BASE_ADDR_MASK) |
      TILE_HEIGHT_8PX |
      TILE_WIDTH_8PX;

  VERA.layer0.hscroll = 0;
  VERA.layer0.vscroll = 0;
}

void main(void)
{
  load_tileset_to_vera();
  setup_tilemap();
  setup_vera();
}
