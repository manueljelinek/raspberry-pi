// Copyright (c) 2014  Manuel Jelinek
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

#include "mailbox.h"


#define MAILBOX_FULL      0x80000000
#define MAILBOX_EMPTY     0x40000000

#define MEMORY_OFFSET     0xC0000000


/* Mailbox memory addresses */
static volatile uint32_t* MAILBOX_READ   = (uint32_t*) 0x2000B880;
static volatile uint32_t* MAILBOX_STATUS = (uint32_t*) 0x2000B898;
static volatile uint32_t* MAILBOX_WRITE  = (uint32_t*) 0x2000B8A0;


uint32_t readMailbox(uint32_t channel)
{
  uint32_t r = 0;
  do {
    while (*MAILBOX_STATUS & MAILBOX_EMPTY); //wait for data
    r = *MAILBOX_READ; // Read the data
  } while ((r & 0xF) != channel); // Loop until we received something from the
  // frame buffer channel
  return r & 0xFFFFFFF0;
}

void writeMailbox(uint32_t channel, uint32_t v)
{
  v += MEMORY_OFFSET;
  while (*MAILBOX_STATUS & MAILBOX_FULL); //wait for space
  // Write the value to the frame buffer channel
  *MAILBOX_WRITE = channel | (v & 0xFFFFFFF0);
}
