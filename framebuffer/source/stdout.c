// Copyright (c) 2014  Manuel Jelinek, Martin Erb
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

#include "stdio.h"
#include "uart.h"
#include "framebuffer.h"  // can be removed if not needed -> all outputs always on uart


/* Defines the used output (UART/Framebuffer) */
static int32_t std_output = OUTPUT_UART;


void setStdOutput(int32_t std_out)
{
  std_output = std_out;
}

int32_t getStdOutput()
{
  return std_output;
}

/**
 * Writes the given character to stdout.
 * The character is casted to unsigned char.
 *
 * @param character The character for writing
 * @return The character written as unsigned char cast to int
 *
 */
int32_t putchar(int32_t character)
{
  uint8_t output_char = (uint8_t)character;
#ifdef FRAMEBUFFER_H
  if(!std_output)
    uartPutc(output_char);
  else
    consoleWriteChar(output_char);
#else
  uartPutc(output_char);
#endif
  return (int32_t)output_char;
}

/**
 * Writes the given string to stdout.
 *
 * @param output_string The string for writing
 * @return A non-negative number on success
 *
 */
int32_t puts(const char* output_string)
{
  int32_t string_length = 0;

  while(output_string && *output_string)
  {
#ifdef FRAMEBUFFER_H
    if(!std_output)
      uartPutc(*output_string++);
    else
      consoleWriteChar(*output_string++);
#else
    uartPutc(*output_string++);
#endif
    ++string_length;
  }

  return string_length;
}
