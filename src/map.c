#include "map.h"

#include <cx16.h>
#include <stdio.h>
#include <stdlib.h>

#include "burning-petscii.h"
#include "vera-util.h"

#define NUM_VALID_MAP_TILES 64

// L = left
// R = right
// T = top
// B = bottom
// M = middle
#define TL 0b10000000
#define TM 0b01000000
#define TR 0b00100000
#define ML 0b00010000
#define MR 0b00001000
#define BL 0b00000100
#define BM 0b00000010
#define BR 0b00000001

// clang-format off
unsigned char tile_outs[64] = {

    // 0x40
    ML | MR,
    0,
    TM | BM,
    ML | MR,
    0,
    0,
    0,
    0,
    0,
    ML | BM,  // 0x49
    TM | MR,
    TM | ML,
    TL | ML | BL | BM | BR,
    TL | BR,
    BR | TL,
    BL | ML | TL | TM | TR,

    // 0x50
    TL | TM | TR | ML | BL,
    0,
    0,
    0,
    0,
    BM | MR,  // 0x55
    TL | TR | BL | BR,  // X
    0,
    0,
    0,
    0,
    TM | ML | MR | BM,  // +-sign
    0,
    0,
    0,
    TL | TM | TR | ML | BL,  // 0x5F TOP RIGHT TRIANGLE

    // 0x60
    0,
    0,
    0,
    0,
    0,
    0,
    TL | BR,  // 0x66
    0,
    0,
    BR | MR | TR | TM | TR,  // 0x69 TOP LEFT TRIANGLE
    0,
    TM | MR | BM,
    BM | BR | MR,
    TM | MR,  // 0x6D
    ML | BM,  // 0x6E
    BL | BM | BR,  // 0x6F

    //0x70
    BM | MR,
    MR | TM | MR,
    MR | BM | MR,
    ML | TM | BM,
    TR | MR | BR,
    TL | ML | BL,  // 0x75
    TR | MR | BR,  // 0x76
    TL | TM | TR,
    TL | TM | TR,
    BL | BM | BR,
    BL | BM | BR | MR | TR,  // 0x7A
    ML | BL | BM,
    TM | TR | MR,
    ML | TM,
    ML | TL | TM,
    ML | TL | TM | MR | BR | BM  // 0x7F
};
// clang-format on

signed char lookback[4][2] = {
    {-1, -1},  // up-left
    {-1, 0},   // up
    {-1, 1},   // up-right
    {0, -1}    // left
};

signed char lookback_row;
signed char lookback_col;
unsigned char lookback_tile;
unsigned char li;
unsigned char cell_connections_mask;
unsigned char tile_to_place;
unsigned char row, col;
unsigned int cells_processed = 0;
unsigned char tile_color = 0xBC;

#define ADJACENCY_CHANCE (RAND_MAX / 100 * 95)
#define NUM_LOOKBACKS 4

void map_setup(void) {
  printf("\nmap  %1c%1c", 30, PROGRESS_BAR_START_CHAR);
 
  VERA.address_hi = (MAP0_ADDR >> 16) | VERA_INC_1;

  for (row = 0; row < MAP_HEIGHT_TILES; row++) {
    for (col = 0; col < MAP_WIDTH_TILES; col++) {
      tile_color = 0xBC;
      if (rand() > ADJACENCY_CHANCE) {
        tile_to_place = (rand() % NUM_VALID_MAP_TILES);
        // tile_color = 0x50;
      } else {
        // compute the connections mask of the cell we are filling
        cell_connections_mask = 0;
        for (li = 0; li < NUM_LOOKBACKS; li++) {
          lookback_row = row + lookback[li][0];
          lookback_col = col + lookback[li][1];
          if (lookback_row < MAP_HEIGHT_TILES && lookback_col < MAP_WIDTH_TILES && lookback_row >= 0 && lookback_col >= 0) {
            // VERA.address = vera_tilemap_addr_offsets[lookback_row][lookback_col];
            lookback_tile = map_tiles_index_get(lookback_row, lookback_col, 0);
            // printf("lookback tile at (%d,%d): %x (%c) with tile_outs[%x]=%x\n", lookback_row, lookback_col, lookback_tile,
            //        lookback_tile + TILE_TO_PETSCII_OFFSET, lookback_tile, tile_outs[lookback_tile]);
            if (li == 0) {  // top left neighbor
              if (tile_outs[lookback_tile] & BR) {
                cell_connections_mask |= TL;
              }
              // printf("TL neighbor connection mask now %x\n", cell_connections_mask);
            } else if (li == 1) {  // top center neighbor
              if (tile_outs[lookback_tile] & BL) {
                cell_connections_mask |= TL;
              }
              if (tile_outs[lookback_tile] & BM) {
                cell_connections_mask |= TM;
              }
              if (tile_outs[lookback_tile] & BR) {
                cell_connections_mask |= TR;
              }
              // printf("TC neighbor connection mask now %x\n", cell_connections_mask);
            } else if (li == 2) {  // top right neighbor
              if (tile_outs[lookback_tile] & BL) {
                cell_connections_mask |= TR;
              }
              // printf("TR neighbor connection mask now %x\n", cell_connections_mask);
            } else if (li == 3) {  // left neighbor
              if (tile_outs[lookback_tile] & TR) {
                // printf("tr ...");
                cell_connections_mask |= TL;
              }
              if (tile_outs[lookback_tile] & MR) {
                // printf("mr ...");
                cell_connections_mask |= ML;
              }
              if (tile_outs[lookback_tile] & BR) {
                // printf("tr ...");
                cell_connections_mask |= BL;
              }
              // printf("left neighbor connection mask now %x\n", cell_connections_mask);
            }
          }
        }
        // printf("cell (%d,%d) connections mask: %x\n", row, col, cell_connections_mask);
        if (cell_connections_mask == 0) {
          tile_to_place = (rand() % NUM_VALID_MAP_TILES);
          tile_color = 0x04;
        } else {
          tile_to_place = 255;
          while (tile_to_place == 255) {
            tile_to_place = (rand() % NUM_VALID_MAP_TILES);
            if (tile_outs[tile_to_place] == 0) {
              tile_to_place = 255;  // try again
            } else if ((tile_outs[tile_to_place] & cell_connections_mask) == 0) {
              tile_to_place = 255;  // try again
            }
          }
        }
      }

      map_tiles_index_set(row, col, 0, tile_to_place);
      map_tiles_index_set(row, col, 1, tile_color);
      cells_processed++;
      if (cells_processed % 165 == 0) {
        printf("%1c%1c%1c", 30, PROGRESS_BAR_CHAR, 5);
      }
    }
  }
  printf("%1c%1c%1c", 30, PROGRESS_BAR_END_CHAR, 5);
}