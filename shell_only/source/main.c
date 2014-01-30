#include <stdint.h>
#include "stdio.h"
#include "stdlib.h"
#include "timer.h"
#include "mmio.h"
#include "uart.h"
#include "string.h"
#include "shell.h"

#define WAIT_DELAY      0x7F0000

void main(void)
{
  char* examplemainhelptext="this is mein main function";
  addNewCommand(examplemaincommandsadd,"startmy",examplemainhelptext);
  printf("Welcome to Martins and Manuels Shell!! \n");
  shell();
}
