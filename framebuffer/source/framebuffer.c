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

#include "framebuffer.h"
#include "mailbox.h"
#include "memutils.h"


/* Framebuffer initialisation failure codes
 * If the FB can't be initialised, one of the following numbers will be
 * flashed on the OK LED
 */
#define FBFAIL_GET_RESOLUTION           1  /* Mailbox call to get screen resolution failed */
#define FBFAIL_GOT_INVALID_RESOLUTION   2  /* Mailbox call returned bad resolution */
#define FBFAIL_SETUP_FRAMEBUFFER        3  /* Mailbox call to setup FB failed */
#define FBFAIL_INVALID_TAGS             4  /* Setup call FB returned an invalid list of response tags */
#define FBFAIL_INVALID_TAG_RESPONSE     5  /* Setup FB call returned an invalid response for the framebuffer tag */
#define FBFAIL_INVALID_TAG_DATA         6  /* Setup FB call returned an invalid address/size */
#define FBFAIL_INVALID_PITCH_RESPONSE   7  /* Read FB pitch call returned an invalid response */
#define FBFAIL_INVALID_PITCH_DATA       8  /* Read FB pitch call returned an invalid pitch value */
#define FBFAIL_INVALID_BIT_SIZE         9

/* Character cells are 8x16 */
#define CHARSIZE_X  8
#define CHARSIZE_Y  16

extern uint8_t font[][CHARSIZE_Y];
extern uint32_t fontsize;  // contains the size of 'font.bin'

/* Screen parameters set in fb_init() */
static uint32_t screenbase, screensize;
static uint32_t fb_x, fb_y;
static uint32_t pitch;  // pitch -> bytes to get one pixel down on monitor
/* Max x/y character cell */
static uint32_t max_chars_x, max_chars_y;
static uint32_t color_mode;

/* Current console text cursor position (ie. where the next character will
 * be written
 */
static int32_t cursor_pos_x = 0;
static int32_t cursor_pos_y = 0;

/* Framebuffer initialisation failed. Can't display an error, so flashing
 * the OK LED will have to do
 */
static void fbFail(uint32_t num)  // get in her if an error occurs
{
  while(1);
    //output(num);
}

/* Initialise the framebuffer with nativ resolution and 16bit color mode */
void fbInitNativ(void)
{
  fbInit(0xFFFFFFFF, 0xFFFFFFFF, COLORMODE_16BIT);
}

/* Initialise the framebuffer */
void fbInit(uint32_t set_fb_x, uint32_t set_fb_y, uint32_t set_color_depth)
{
  
  cursor_pos_x = 0;
  cursor_pos_y = 0;
  uint32_t var;
  uint32_t count;

  /* min resolution */
  if(set_fb_x < 640)
    set_fb_x = 640;
  if(set_fb_y < 480)
    set_fb_y = 480;

  /* 16bit color mode, if no supported color is set */
  if(set_color_depth < 2 || set_color_depth > 4)
    color_mode = COLORMODE_16BIT;
  else
    color_mode = set_color_depth;

  volatile uint32_t mailbuffer[256] __attribute__((aligned (0x10)));

  /* Get the display size */
  mailbuffer[0] = 8 * 4;    // Total size to send via Mailbox
  mailbuffer[1] = 0;        // Type: Request
  mailbuffer[2] = 0x40003;  // Tag ID: physical display size
  mailbuffer[3] = 8;        // Buffer size
  mailbuffer[4] = 0;        // Request size
  mailbuffer[5] = 0;        // Space for horizontal resolution
  mailbuffer[6] = 0;        // Space for vertical resolution
  mailbuffer[7] = 0;        // End tag

  writeMailbox(GPU_MAILBOX_CHANNEL, (uint32_t)mailbuffer);
  var = readMailbox(GPU_MAILBOX_CHANNEL);

  /* Valid response in data structure */
  if(mailbuffer[1] != 0x80000000)
    fbFail(FBFAIL_GET_RESOLUTION);

  fb_x = mailbuffer[5];
  fb_y = mailbuffer[6];

  /* If both fb_x and fb_y are both zero, assume we're running on the
   * qemu Raspberry Pi emulation (which doesn't return a screen size
   * at this point), and request a 640x480 screen
   */
  if(fb_x==0 && fb_y==0)
  {
    fb_x = 640;
    fb_y = 480;
  }

  if(fb_x==0 || fb_y==0)
    fbFail(FBFAIL_GOT_INVALID_RESOLUTION);

  /* check if wished resolution larger then nativ */
  if(set_fb_x < fb_x)
    fb_x = set_fb_x;
  if(set_fb_y < fb_y)
    fb_y = set_fb_y;

  /* Set up screen */
  uint32_t c = 1;
  mailbuffer[c++] = 0;    // Request

  mailbuffer[c++] = 0x00048003; // Tag id (set physical size)
  mailbuffer[c++] = 8;          // Value buffer size (bytes)
  mailbuffer[c++] = 8;          // Req. + value length (bytes)
  mailbuffer[c++] = fb_x;       // Horizontal resolution
  mailbuffer[c++] = fb_y;       // Vertical resolution

  mailbuffer[c++] = 0x00048004; // Tag id (set virtual size)
  mailbuffer[c++] = 8;          // Value buffer size (bytes)
  mailbuffer[c++] = 8;          // Req. + value length (bytes)
  mailbuffer[c++] = fb_x;       // Horizontal resolution
  mailbuffer[c++] = fb_y;       // Vertical resolution

  mailbuffer[c++] = 0x00048005; // Tag id (set depth)
  mailbuffer[c++] = 4;          // Value buffer size (bytes)
  mailbuffer[c++] = 4;          // Req. + value length (bytes)
  mailbuffer[c++] = color_mode * 8;  // color bit depth (can be 16, 24 or 32bit)

  mailbuffer[c++] = 0x00040001; // Tag id (allocate framebuffer)
  mailbuffer[c++] = 8;          // Value buffer size (bytes)
  mailbuffer[c++] = 4;          // Req. + value length (bytes)
  mailbuffer[c++] = 16;         // Alignment = 16
  mailbuffer[c++] = 0;          // Space for response

  mailbuffer[c++] = 0;          // Terminating tag

  mailbuffer[0] = c*4;          // Buffer size

  writeMailbox(GPU_MAILBOX_CHANNEL, (uint32_t)mailbuffer);
  var = readMailbox(GPU_MAILBOX_CHANNEL);

  /* Valid response in data structure */
  if(mailbuffer[1] != 0x80000000)
    fbFail(FBFAIL_SETUP_FRAMEBUFFER);

  count=2;  /* First tag */
  while((var = mailbuffer[count]))
  {
    if(var == 0x40001)
      break;

    /* Skip to next tag
     * Advance count by 1 (tag) + 2 (buffer size/value size)
     *                          + specified buffer size
     */
    count += 3+(mailbuffer[count+1]>>2);

    if(count>c)
      fbFail(FBFAIL_INVALID_TAGS);
  }

  /* 8 bytes, plus MSB set to indicate a response */
  if(mailbuffer[count+2] != 0x80000008)
    fbFail(FBFAIL_INVALID_TAG_RESPONSE);

  /* Framebuffer address/size in response */
  screenbase = mailbuffer[count+3];
  screensize = mailbuffer[count+4];

  if(screenbase == 0 || screensize == 0)
    fbFail(FBFAIL_INVALID_TAG_DATA);

  /* Get the framebuffer pitch (bytes per line) */
  mailbuffer[0] = 7 * 4;    // Total size
  mailbuffer[1] = 0;        // Request
  mailbuffer[2] = 0x40008;  // Display size
  mailbuffer[3] = 4;        // Buffer size
  mailbuffer[4] = 0;        // Request size
  mailbuffer[5] = 0;        // Space for pitch
  mailbuffer[6] = 0;        // End tag

  writeMailbox(GPU_MAILBOX_CHANNEL, (uint32_t)mailbuffer);
  var = readMailbox(GPU_MAILBOX_CHANNEL);

  /* 4 bytes, plus MSB set to indicate a response */
  if(mailbuffer[4] != 0x80000004)
    fbFail(FBFAIL_INVALID_PITCH_RESPONSE);

  pitch = mailbuffer[5];
  if(pitch == 0)
    fbFail(FBFAIL_INVALID_PITCH_DATA);

  /* Need to set up max_chars_x/max_chars_y before using console_write */
  max_chars_x = fb_x / CHARSIZE_X;
  max_chars_y = fb_y / CHARSIZE_Y;
  color_mode = pitch / fb_x;

  if(color_mode < 2 || color_mode > 4)
    fbFail(FBFAIL_INVALID_BIT_SIZE);
}

/* Current fg/bg color, this initialization is correct vor all bit modes */
static rgb rgb_foreground = {.rgb_ = COLOR32_WHITE};
static rgb rgb_background = {.rgb_ = COLOR32_BLACK};

/* Move to a new line, and, if at the bottom of the screen, scroll the
 * framebuffer 1 character row upwards, discarding the top row
 */
static void newline(int32_t new_cursor_pos_x)
{
  cursor_pos_x = new_cursor_pos_x;
  if(cursor_pos_y<(max_chars_y-1))
  {
    ++cursor_pos_y;
    return;
  }

  /* Copy a screen's worth of data (minus 1 character row) from the
   * second row to the first
   */

  uint32_t source;
  /* Number of bytes in a character row */
  uint32_t rowbytes = CHARSIZE_Y * pitch;

  /* Calculate the address to copy the screen data from */
  source = screenbase + rowbytes;
  memmove((void*)screenbase, (void*)source, (max_chars_y-1) * rowbytes);

  /* Clear last line on screen */
  memclr((void*)(screenbase + (max_chars_y-1)*rowbytes), rowbytes);
}

static void writeField(uint8_t character)
{
  uint32_t row;
  int32_t  col;

  uint32_t field_start_addr = screenbase + cursor_pos_y*CHARSIZE_Y*pitch + cursor_pos_x*CHARSIZE_X*color_mode;
  switch(color_mode)
  {
    /* 16bit color mode */
    case COLORMODE_16BIT:
    {
      volatile rgb16* ptr;

      ptr = (rgb16*)field_start_addr;
      for(row=0; row<CHARSIZE_Y; ++row)
      {
        for(col=0; col<CHARSIZE_X; ++col)
        {
          if(font[character][row] & (0x80>>col))
            ptr[col] = rgb_foreground.rgb_16;
          else
            ptr[col] = rgb_background.rgb_16;
        }

        ptr += fb_x;
      }
      break;
    }

    /* 24bit color mode */
    case COLORMODE_24BIT:
    {
      volatile rgb24* ptr;

      ptr = (rgb24*)field_start_addr;
      for(row=0; row<CHARSIZE_Y; ++row)
      {
        for(col=0; col<CHARSIZE_X; ++col)
        {
          if(font[character][row] & (0x80>>col))
          {
            ptr[col].r = rgb_foreground.rgb_24.r;
            ptr[col].g = rgb_foreground.rgb_24.g;
            ptr[col].b = rgb_foreground.rgb_24.b;
          }
          else
          {
            ptr[col].r = rgb_background.rgb_24.r;
            ptr[col].g = rgb_background.rgb_24.g;
            ptr[col].b = rgb_background.rgb_24.b;
          }
        }

        ptr += fb_x;
      }
      break;
    }

    /* 32bit color mode */
    case COLORMODE_32BIT:
    {
      volatile rgb32* ptr;

      ptr = (rgb32*)field_start_addr;
      for(row=0; row<CHARSIZE_Y; ++row)
      {
        for(col=0; col<CHARSIZE_X; ++col)
        {
          if(font[character][row] & (0x80>>col))
            ptr[col] = rgb_foreground.rgb_32;
          else
            ptr[col] = rgb_background.rgb_32;
        }

        ptr += fb_x;
      }
      break;
    }

    default:
      fbFail(FBFAIL_INVALID_BIT_SIZE);
  }
}

static void oneFieldBack()
{
  if(!cursor_pos_x)
  {
    if(!cursor_pos_y)  // alreade on first line, first sign ;o)
      return;
    cursor_pos_x = max_chars_x-1;
    --cursor_pos_y;
  }
  else --cursor_pos_x;
}

static void clearScreen(void)
{
  cursor_pos_x = 0;
  cursor_pos_y = 0;

  /* Clear all lines on screen */
  memclr((void*)screenbase, fb_y * pitch);
}

void consoleWriteChar(uint8_t character)
{
  switch(character)
  {
    // '\0' NULL:
    case  0x0: return;

    // '\b' Backspace
    case  0x8:
      oneFieldBack();
      return;

    // '\t' horizontal tab
    case  0x9:
      do {
        if(++cursor_pos_x >= max_chars_x) newline(0);
      } while((cursor_pos_x) & 7);
      return;

    // '\n' newline
    case  0xA:
      newline(0);
      return;

    // '\v' vertical tab
    case  0xB:
      newline(cursor_pos_x);
      return;

    // '\f' form feed (clears screen)
    case  0xC:
      clearScreen();
      return;

    // '\r' carriage return
    case  0xD: return;

    // ASCII 'DEL'
    case 0x7F:
      oneFieldBack();
      writeField(' '); /* Write a Space to clear the Field */
      return;
  }

  writeField(character);

  if(++cursor_pos_x >= max_chars_x)
    newline(0);
}

void consoleForegroundColor(rgb new_fg_color)
{
  rgb_foreground = new_fg_color;
}

void consoleBackgroundColor(rgb new_bg_color)
{
  rgb_background = new_bg_color;
}

