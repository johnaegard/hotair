#include <cx16.h>
#include <cbm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TILESET_VERA_ADDR 0x1F000
#define TILEMAP_VERA_ADDR 0x0000
#define TILESET_FILE "assets/PETSCII.BIN"
#define TILESET_SIZE 4096  // 256 tiles * 16 bytes per tile

void load_tileset_to_vera(void) {
    FILE *fp;
    unsigned char buffer[256];
    unsigned int bytes_read;
    unsigned long vera_addr = TILESET_VERA_ADDR;

    printf("Loading tileset from %s...\n", TILESET_FILE);
    
    fp = fopen(TILESET_FILE, "rb");
    if (fp == NULL) {
        printf("Error: Could not open %s\n", TILESET_FILE);
        return;
    }

    // Set up VERA address for tileset
    VERA.address = vera_addr & 0xFFFF;
    VERA.address_hi = (vera_addr >> 16) | 0x10;  // VERA_INC_1
    
    while ((bytes_read = fread(buffer, 1, 256, fp)) > 0) {
        for (unsigned int i = 0; i < bytes_read; i++) {
            VERA.data0 = buffer[i];
        }
    }
    
    fclose(fp);
    printf("Tileset loaded successfully!\n");
}

void setup_tilemap(void) {
    unsigned int tile_index = 0;
    unsigned int x, y;
    
    printf("Setting up 16x16 tilemap...\n");
    
    // Set up VERA address for tilemap
    VERA.address = TILEMAP_VERA_ADDR & 0xFFFF;
    VERA.address_hi = (TILEMAP_VERA_ADDR >> 16) | 0x10;  // VERA_INC_1
    
    // Create a 16x16 matrix of tiles (0-255)
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            VERA.data0 = tile_index & 0xFF;          // tile index low byte
            VERA.data0 = (tile_index >> 8) & 0x0F;   // tile index high nibble + flags
            tile_index++;
        }
        // Skip the rest of the tilemap row (64 tiles per row)
        for (x = 16; x < 64; x++) {
            VERA.data0 = 0;
            VERA.data0 = 0;
        }
    }
    
    printf("Tilemap configured!\n");
}

void setup_vera_layer1(void) {
    printf("Configuring VERA layer 1...\n");
    
    // VERA layer 1 control register (0x0F000)
    VERA.address = 0x0000;
    VERA.address_hi = 0x10;  // 0x0F000 in high
    VERA.address_hi |= 0x80; // Set addressing to 0x0Fxxx
    
    // Configure Layer 1 (16x16 tiles, 256-color)
    VERA.data0 = 0x00;  // Layer 1 Map Base: 0x00000
    
    // Layer 1 Tile Base: point to our tileset at 0x1F000
    VERA.data0 = 0xF8;  // 0x1F000 >> 8 (upper bits)
    
    // Layer 1 Control: 16x16 tiles, 256-color mode
    VERA.data0 = 0x00;  // 0 = 256 tiles per row (16x16), 0 = 256-color
    
    // Layer 1 H Scroll and V Scroll both 0
    VERA.data0 = 0x00;
    VERA.data0 = 0x00;
    VERA.data0 = 0x00;
    VERA.data0 = 0x00;
    
    printf("VERA layer 1 configured!\n");
}

void main(void) {
    load_tileset_to_vera();
    setup_tilemap();
    setup_vera_layer1();
    
    printf("\nPress any key to exit...\n");
    cgetc();
    
    printf("Goodbye!\n");
}
