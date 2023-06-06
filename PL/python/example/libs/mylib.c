#include "mylib.h"

#include <stdio.h>

void mylib_printf(char* str) {
  printf("called by python\n");
  printf("%s\n", str);
}

// gcc -g -shared -fPIC libs/mylib.c -o libs/mylib.so