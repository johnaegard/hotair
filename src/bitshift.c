#include <stdio.h>

signed char foo = -65;
unsigned char charset = 2;

void main() {
   
  asm("lda #2");
  asm("jsr $FF62");

  printf("%d\n",foo);

  printf("%d\n",foo >> 6);
  printf("%d\n",foo / 64);

  printf("sizeof(signed short):%d\n",sizeof(signed short));
  printf("sizeof(unsigned short):%d\n",sizeof(unsigned short));

  printf("sizeof(signed int):%d\n",sizeof(signed int));
  printf("sizeof(unsigned int):%d\n",sizeof(unsigned int));

  printf("sizeof(signed long):%d\n",sizeof(signed long));
  printf("sizeof(unsigned long):%d\n",sizeof(unsigned long));

}