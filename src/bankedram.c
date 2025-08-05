#include <cx16.h>
#include <string.h>
#include <stdio.h>

#define BANK_NUM (*(unsigned char *)0x00)
#define ARRAY_2D_ADDRESS 0xA000

char (*array_2d)[64] = (char (*)[64])ARRAY_2D_ADDRESS;

void main(void) {

// LDA #$80
// CLC
// JSR screen_mode ; SET 320x240@256C MODE
// BCS FAILURE

    asm("lda #$00");
    asm("clc");
    asm("jsr $FF5F");  // screen mode 80
    asm("jsr $FECF");  // 
    
    BANK_NUM=1;
    array_2d[32][32] = 1;
    
    BANK_NUM=2;
    array_2d[32][32] = 2;

    BANK_NUM=1;
    printf("bank 1 array_2d[32][32]: %d\n", array_2d[32][32]);

    BANK_NUM=2;
    printf("bank 2 array_2d[32][32]: %d\n", array_2d[32][32]);

    BANK_NUM=1;
    printf("bank 1 array_2d[32][32]: %d\n", array_2d[32][32]);

    BANK_NUM=2;
    printf("bank 2 array_2d[32][32]: %d\n", array_2d[32][32]);

}