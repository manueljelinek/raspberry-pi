#include <stdint.h>
#include "framebuffer.h"
#include "stdio.h"
#include "stdlib.h"
#include "timer.h"
#include "mmio.h"
#include "uart.h"
#include "string.h"
#include "shell.h"

#define WAIT_DELAY      0x7F0000

void writeTextInBox(const char* text)  // only one Line (don't use newlines)
{
  uint32_t text_length = strlen(text) + 2;
  int32_t i;
  putchar(0xC9); for(i=0; i<text_length; ++i) putchar(0xCD); putchar(0xBB); putchar('\n');
  putchar(0xBA); putchar(' '); printf(text);  putchar(' '); putchar(0xBA); putchar('\n');
  putchar(0xC8); for(i=0; i<text_length; ++i) putchar(0xCD); putchar(0xBC); putchar('\n');
}

void main(void)
{
  uartInit();
  char* myhelp="this is mein main function";
  addNewCommand(newcommandstoadd,"startmy",myhelp);

// -----
  // Initialise framebuffer with maximum resolution of monitor and 16bit colordepth
  fbInitNativ();

// -----
  // Initialise framebuffer with choosen resolution:
  //uint32_t fb_x = 640;
  //uint32_t fb_y = 480;
  //fbInit(fb_x, fb_y, COLORMODE_32BIT);

// -----
  // Change standard output from uart to monitor:
  setStdOutput(OUTPUT_MONITOR);
  //setStdOutput(OUTPUT_UART);  // set as default

  // Only a small output on monitor to say hello ;)
  writeTextInBox("Welcome to Martins and Manuels Shell!!");

// -----
// Examples what you can do with the framebuffer / only working with framebuffer
// -----

  // change foreground/background color (example)
  //rgb foreground = {.rgb_16.rgb = COLOR16_RED};
  //rgb background = {.rgb_16.rgb = COLOR16_YELLOW};
  //rgb foreground = {.rgb_ = COLOR32_RED};
  //rgb background = {.rgb_ = COLOR32_YELLOW};
  //consoleForegroundColor(foreground);
  //consoleBackgroundColor(background);

// -----
  // Only some examples for dynamic memory allocation:
/*
  printf("start address of heap: %x\n", &_heap_start);

  uint32_t* test1 = malloc(5*sizeof(uint32_t));
  printf("test1: %x\n", test1);
  uint32_t* test2 = malloc(10*sizeof(uint32_t));
  //test2[10] = 55;  // to test an error
  printf("test2: %x\n", test2);
  uint32_t* test3 = malloc(20*sizeof(uint32_t));
  printf("test3: %x\n", test3);

  free(test2);
  test2 = malloc(3*sizeof(uint32_t));
  printf("test2: %x\n", test2);
  uint32_t* test4 = malloc(2*sizeof(uint32_t));
  printf("test4: %x\n", test4);
  free(test4);
  test2 = realloc(test2, 10*sizeof(uint32_t));
  printf("test2: %x\n", test2);
  test2 = realloc(test2, 11*sizeof(uint32_t));
  printf("test2: %x\n", test2);
  test4 = malloc(5*sizeof(uint32_t));
  printf("test4: %x\n", test4);
*/


// -----
  // More printable signs available on monitor output ;)
  //int32_t i;
  //for(i=128;i<256;++i)
  //{
  //  if(!(i & 0xF)) putchar('\n');
  //  putchar(i);
  //}
  //putchar('\n');

// -----
  // Start the shell:
  shell();
}
