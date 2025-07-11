#include <stdio.h>

void main() {

  signed char foo = -65;
  printf("%d\n",foo);

  printf("%d\n",foo >> 6);
  printf("%d\n",foo / 64);

}