#include <cx16.h>
#include <cbm.h>
#include <stdio.h>

#define R0 (*(unsigned short *)0x02)
#define R1 (*(unsigned short *)0x04)
#define R2 (*(unsigned short *)0x06)
char petscii[2048];
char filename[11] = "petscii.bin";
unsigned long src_vram_addr = 0x1F000;
char file_error_num;

void main(void){

  asm("lda #2");
  asm("jsr $FF62");

  VERA.address = src_vram_addr;
  VERA.address_hi = src_vram_addr >> 16;
  VERA.address_hi |= 0b10000; // Increment mode

  R0 = 0x9F23; 
  R1 = (unsigned short) petscii;
  R2 = 2048;

  // Call the memory_copy Kernal Function
  __asm__("jsr $FEE7");

  cbm_k_setnam(filename);

  cbm_k_setlfs(0, 8, 1);

  cbm_k_save((unsigned int) petscii, (unsigned int)(petscii + 2048));

  file_error_num = 0;
  __asm__("bcc noerror");
  __asm__("lda #1");
  __asm__("sta %v", file_error_num);
  __asm__("noerror:");

  if (file_error_num) {
    printf("err#%02u%1c\n", file_error_num, 5);
  }

}