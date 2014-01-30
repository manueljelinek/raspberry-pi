#include <stdint.h>
#include "stdio.h"
#include "stdlib.h"
#include "timer.h"
#include "mmio.h"
#include "uart.h"
#include "string.h"
#include "shell.h"


void examplemain(void)
{
  printf("now executing examplemain\n");
}

void examplemaincommandsadd() {
  addNewCommand(examplemain,"startexamplemain",NULL);
}
