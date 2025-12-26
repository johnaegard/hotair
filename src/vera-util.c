#include <cx16.h>
#include <cbm.h>
#include <stdio.h>
#include <stdlib.h>
#include "vera-util.h"

unsigned char file_error_num=0;

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