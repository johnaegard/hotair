#include <cx16.h>
#include <stdlib.h>
#include <stdio.h>
#include "burning-petscii.h"
#include "vera-util.h"

#define VALID_MAP_TILES_START_INDEX 64
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

unsigned char tile_outs[64] = {
    ML | MR,  // 0x40
    255,       
    TM | BM, 
    ML | MR, 
    255, 
    255, 
    255, 
    255,  
    255,
    ML | BM,  // 0x49
    TM | MR,
    TM | ML,
    TL | ML | BL | BM | BR,
    TL | BR,
    BR | TL,
    BL | ML | TL | TM | TR,

    TL | TM | TR | ML | BL, // 0x50
    255,
    255,
    255,
    255,
    BM | MR,  // 0x55
    TL | TR | BL | BR, // X
    255,
    255,
    255,
    255,
    TM | ML | MR | BM, // +-sign
    255,
    255,
    255,
    TL | TM | TR | ML | BL,  // 0x5F TOP RIGHT TRIANGLE
    
    255,
    255,
    255,
    255,
    255,
    255,
    TL | BR, //0x66
    255,
    255,
    BR | MR | TR | TM | TR,  // 0x69 TOP LEFT TRIANGLE
    255,
    TM | MR | BM,
    BM | BR | MR,
    TM | MR,
    ML | BM,
    BL | BM | BR,  // 0x6F

    BM | MR, 
    MR | TM | MR,
    MR | BM | MR,
    ML | TM | BM,
    TR | MR | BR,
    TR | MR | BR,
    TL | ML | BL, // 0x76
    TL | TM | TR, 
    TL | TM | TR, 
    BL | BM | BR,
    BL | BM | BR | MR | TR,  // 0x7A
    ML | BL | BM,
    TM | TR | MR,
    ML | TM, 
    ML | TL | TM,
    ML | TL | TM | MR | BR | BM // 0x7F
};

#define ADJACENCY_CHANCE 32000
#define NUM_LOOKBACKS 4

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
unsigned int cells_processed =0;

void map_setup(void) {
  printf("\nseeding map:  ");

  VERA.address_hi = MAP0_ADDR | VERA_INC_1;
  for (row = 0; row < MAP_HEIGHT_TILES; row++) {
    for (col = 0; col < MAP_WIDTH_TILES; col++) {
      if (rand() < ADJACENCY_CHANCE) {

        // compute the connections mask of the cell we are filling
        cell_connections_mask = 0;
        for (li = 0; li < NUM_LOOKBACKS; li++) {
          lookback_row = row + lookback[li][0];
          lookback_col = col + lookback[li][1];
          if (lookback_row < MAP_HEIGHT_TILES &&
              lookback_col < MAP_WIDTH_TILES && lookback_row >= 0 &&
              lookback_col >= 0) {
            VERA.address = vera_tilemap_addr_offsets[lookback_row][lookback_col];
            lookback_tile = VERA.data0 - VALID_MAP_TILES_START_INDEX;
            if (li == 0) {  // top left neighbor
              if (tile_outs[lookback_tile] && BR) {
                cell_connections_mask |= TL;
              }
            }
            else if ( li == 1) { // top center neighbor
              if (tile_outs[lookback_tile] && BL) {
                cell_connections_mask |= TL;
              }
              if (tile_outs[lookback_tile] && BM) {
                cell_connections_mask |= TM;
              }
              if (tile_outs[lookback_tile] && BR) {
                cell_connections_mask |= TR;
              }
            }
            else if ( li == 2) { // top right neighbor
              if (tile_outs[lookback_tile] && BL) {
                cell_connections_mask |=TR;
              }
            }
            else if (li == 3 ) { // left neighbor
              if (tile_outs[lookback_tile] && TR) {
                cell_connections_mask |= TL;
              }
              if (tile_outs[lookback_tile] && MR) {
                cell_connections_mask |= ML;
              }
              if (tile_outs[lookback_tile] && BR) {
                cell_connections_mask |= BL;
              }
            }
          }
        }
        if (cell_connections_mask == 0) { 
          tile_to_place = (rand() % NUM_VALID_MAP_TILES) + VALID_MAP_TILES_START_INDEX;
        }
        else {
          tile_to_place = 255; 
          while (tile_to_place == 255) {
            tile_to_place = (rand() % NUM_VALID_MAP_TILES) + VALID_MAP_TILES_START_INDEX;
            if (tile_outs[tile_to_place] == 255) {
              tile_to_place = 255; // try again
            }
            else if ((tile_outs[tile_to_place] & cell_connections_mask) == 0) {
              tile_to_place = 255; // try again
            }
          }
        }
      }
      else {
        tile_to_place = (rand() % NUM_VALID_MAP_TILES) + VALID_MAP_TILES_START_INDEX;
      }

      VERA.address = vera_tilemap_addr_offsets[row][col];
      VERA.data0 = tile_to_place;  // space char
      VERA.data0 = 0x01; 
      cells_processed++;
      if (cells_processed % 2000 == 0) {
        // printf("%1c%1c%1c", 30, 0x63, 5);
      }
    }
  }
}