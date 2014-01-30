// Copyright (c) 2014  Martin Erb
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to
// do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "shell.h"
#include "uart.h"
#include "stdio.h"
#include "string.h"

typedef struct 
{
  fcn_ptr command;
  char command_name[24];
  char* help_text;
} command_struct;

command_struct allcommands[MAXNUMBEROFFUNCTIONS];
int commandcounter = 0;

void helpOutput();

void shell(void)
{
  uartInit();
  char* help_test = "print a helpoutput for all commands\n";
  addNewCommand(helpOutput,"help",help_test);
  printf("Starting Shell!!\n");

  char* shell_prompt = "cmd> ";
  uint32_t exit = 1;
  int32_t len = 0;
  char input[64];
  uint8_t command_to_execute =0;
  uint32_t counter = 0;
  while(exit)
  {
    printf("%s",shell_prompt);
    do
    {
      input[len] = uartGetc();
      putchar(input[len]);
      if(input[len] == 0x7F){
        input[len] = 0;
        len -=2;
      }
    } while(input[len++] != '\n');
    input[--len] = 0;
    if (strcmp(input,"exit") == 0){exit = 0;}
    else if(strcmp(input,"restart") == 0){ printf("restarting...\n");__asm__ ("b _start"); }
    else if(strcmp(input,"shutdown")== 0){ printf("shutdown...\n"); uartPuts("\x04\x04\x04");}
    else if(strcmp(input,"load") == 0){ printf("load new kernel...\n");__asm__ ("ldr r1,=_jump_to_raspbootin \r\n ldr r0,[r1] \r\nblx r0");}
    else
    {
      while(counter < commandcounter){
        if(strcmp(input,allcommands[counter].command_name)==0)
        {
          allcommands[counter].command();
          command_to_execute = 1;
          break;
        }
        counter++;
      }
      if(command_to_execute == 0)
      {
        printf("Unknwon Command!\n");
      }
    }
    len=0;
    counter =0;
    command_to_execute =0;
  }
  printf("Close shell\n");
}

void addNewCommand(fcn_ptr function_pointer, char command_name[24], char* help_text)
{
  allcommands[commandcounter].command = function_pointer;
  int i;
  for( i= 0; i < 24 ;i++)
  {
    allcommands[commandcounter].command_name[i] = command_name[i];
  }
  allcommands[commandcounter].help_text = help_text;
  commandcounter++;
}

void helpOutput()
{
  printf("-----------------------------------------\n");
  printf("Helpfunction\n");
  printf("The following Commands are availabel now!\n");
  printf("exit:\t\tclose the shell and returns to main\n");
  printf("restart:\trestart the program at the mainfunction\n");
  printf("shutdown:\tclose the program and stop raspbootcom\n");
  printf("load:\t\tstart raspbootin and reload the kernel\n");
  int i;
  for(i = 0; i < commandcounter; i ++)
  {
    if(allcommands[i].help_text != 0)
    {
      printf("%s:\t\t%s\n",allcommands[i].command_name,allcommands[i].help_text);
    }
  }

  printf("\n");
} 

