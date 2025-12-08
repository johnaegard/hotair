#include <cx16.h>
#include <string.h>
#include <stdio.h>

#define BANK_NUM (*(unsigned char *)0x00)
#define ARRAY_2D_ADDRESS 0xA000

char (*array_2d)[64] = (char (*)[64])ARRAY_2D_ADDRESS;

void main(void) {
    
    asm("lda #$01");
    asm("clc");
    asm("jsr $FF5F");     // set screen mode and clear

    BANK_NUM=1;
    array_2d[127][63] = 127;
    
    BANK_NUM=2;
    array_2d[127][63] = 255;

    BANK_NUM=1;
    printf("bank 1 array_2d[127][63]: %d\n", array_2d[127][63]);

    BANK_NUM=2;
    printf("bank 2 array_2d[127][63]: %d\n", array_2d[127][63]);

    BANK_NUM=1;
    printf("bank 1 array_2d[127][63]: %d\n", array_2d[127][63]);

    BANK_NUM=2;
    printf("bank 2 array_2d[127][63]: %d\n", array_2d[127][63]);

}