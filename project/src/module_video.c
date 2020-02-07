// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_kernel.h"
#include "module_terminal.h"
#include "module_keyboard.h"
#include "module_video.h"
#include "module_video_mode.h"
#include "module_video_font.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//#define printf(V) module_terminal_global_print_c_string(V)
#define inportb(a)  module_kernel_in_8(a)
#define outportb(a,b)  module_kernel_out_8(a,b)
#define memset(a,b,c)  module_kernel_memset(a,b,c)
#define memcpy(a,b,c)  module_kernel_memcpy(a,b,c)
module_heap_heap_bm *module_video_heap;
#define kmalloc(a) module_heap_alloc(module_video_heap, a)

// adapted code, thanks for Chris Giese <geezer@execpc.com> http://my.execpc.com/~geezer

#define RGB_TO_332(R,G,B)  ((R & 0xE0) | ((G  >> 3 ) & 0x1C) | (B >> 6))

#define  VGA_AC_INDEX      0x3C0
#define  VGA_AC_WRITE      0x3C0
#define  VGA_AC_READ       0x3C1
#define  VGA_MISC_WRITE    0x3C2
#define VGA_SEQ_INDEX      0x3C4
#define VGA_SEQ_DATA      0x3C5
//#define  VGA_DAC_READ_INDEX  0x3C7
#define  VGA_DAC_WRITE_INDEX  0x3C8
#define  VGA_DAC_DATA      0x3C9
#define  VGA_MISC_READ      0x3CC
#define VGA_GC_INDEX       0x3CE
#define VGA_GC_DATA       0x3CF
//      COLOR emulation    MONO emulation
#define VGA_CRTC_INDEX    0x3D4    // 0x3B4
#define VGA_CRTC_DATA      0x3D5    // 0x3B5
#define  VGA_INSTAT_READ    0x3DA

#define  VGA_NUM_SEQ_REGS  5
#define  VGA_NUM_CRTC_REGS  25
#define  VGA_NUM_GC_REGS    9
#define  VGA_NUM_AC_REGS    21
//#define  VGA_NUM_REGS      (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
  VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

void vga_set_graphic();
// uint32_t vga_width();
// uint32_t vga_height();
void vga_flip();
void vga_clear(uint8_t c);
void (*vga_write_pixel)(unsigned x, unsigned y, unsigned char c);
//void vga_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
//  uint8_t color);
void vga_putchar(uint32_t x, uint32_t y, uint8_t fgcolor, uint8_t bgcolor,
  const char c);
uint32_t width, height, depth;
uint8_t* fb;
//void (*putpixel)(unsigned x, unsigned y, unsigned c);
void vga_set_RRRGGGBB();

//static unsigned g_wd, g_ht;

// -------------------------------------------------------------------------- //
/*
void read_regs(unsigned char *regs)
{
  unsigned i;

// read MISCELLANEOUS reg
  *regs = inportb(VGA_MISC_READ);
  regs++;
// read SEQUENCER regs
  for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
  {
    outportb(VGA_SEQ_INDEX, i);
    *regs = inportb(VGA_SEQ_DATA);
    regs++;
  }
// read CRTC regs
  for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
  {
    outportb(VGA_CRTC_INDEX, i);
    *regs = inportb(VGA_CRTC_DATA);
    regs++;
  }
// read GRAPHICS CONTROLLER regs
  for(i = 0; i < VGA_NUM_GC_REGS; i++)
  {
    outportb(VGA_GC_INDEX, i);
    *regs = inportb(VGA_GC_DATA);
    regs++;
  }
// read ATTRIBUTE CONTROLLER regs
  for(i = 0; i < VGA_NUM_AC_REGS; i++)
  {
    (void)inportb(VGA_INSTAT_READ);
    outportb(VGA_AC_INDEX, i);
    *regs = inportb(VGA_AC_READ);
    regs++;
  }
// lock 16-color palette and unblank display
  (void)inportb(VGA_INSTAT_READ);
  outportb(VGA_AC_INDEX, 0x20);
}
*/
// -------------------------------------------------------------------------- //
//! Used to switch between graphics and text modes.
void write_regs(unsigned char *regs)
{
  unsigned i;

// write MISCELLANEOUS reg
  outportb(VGA_MISC_WRITE, *regs);
  regs++;
// write SEQUENCER regs
  for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
  {
    outportb(VGA_SEQ_INDEX, i);
    outportb(VGA_SEQ_DATA, *regs);
    regs++;
  }
// unlock CRTC registers
  outportb(VGA_CRTC_INDEX, 0x03);
  outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) | 0x80);
  outportb(VGA_CRTC_INDEX, 0x11);
  outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) & ~0x80);
// make sure they remain unlocked
  regs[0x03] |= 0x80;
  regs[0x11] &= ~0x80;
// write CRTC regs
  for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
  {
    outportb(VGA_CRTC_INDEX, i);
    outportb(VGA_CRTC_DATA, *regs);
    regs++;
  }
// write GRAPHICS CONTROLLER regs
  for(i = 0; i < VGA_NUM_GC_REGS; i++)
  {
    outportb(VGA_GC_INDEX, i);
    outportb(VGA_GC_DATA, *regs);
    regs++;
  }
// write ATTRIBUTE CONTROLLER regs
  for(i = 0; i < VGA_NUM_AC_REGS; i++)
  {
    (void)inportb(VGA_INSTAT_READ);
    outportb(VGA_AC_INDEX, i);
    outportb(VGA_AC_WRITE, *regs);
    regs++;
  }
// lock 16-color palette and unblank display
  (void)inportb(VGA_INSTAT_READ);
  outportb(VGA_AC_INDEX, 0x20);
}
// -------------------------------------------------------------------------- //
static void set_plane(unsigned p)
{
  unsigned char pmask;
  p &= 3;
  pmask = 1 << p;
  // set read plane
  outportb(VGA_GC_INDEX, 4);
  outportb(VGA_GC_DATA, p);
  // set write plane
  outportb(VGA_SEQ_INDEX, 2);
  outportb(VGA_SEQ_DATA, pmask);
}
// -------------------------------------------------------------------------- //
static unsigned vga_get_fb(void)
{
  unsigned seg;
  outportb(VGA_GC_INDEX, 6);
  seg = inportb(VGA_GC_DATA);
  seg >>= 2;
  seg &= 3;
  switch(seg)
  {
  case 0:
  case 1:
    seg = 0xA0000;
    break;
  case 2:
    seg = 0xB0000;
    break;
  case 3:
    seg = 0xB8000;
    break;
  }
  return seg;
}
// -------------------------------------------------------------------------- //
static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count)
{
  unsigned i = 0;
  unsigned char *fb = vga_get_fb();
  for (i = 0; i < count; i++)
  {
    fb[dst_off+i] = src[i];
  }
}
// -------------------------------------------------------------------------- //
/*
// -------------------------------------------------------------------------- //
// write font to plane P4 (assuming planes are named P1, P2, P4, P8)
// -------------------------------------------------------------------------- //
static void write_font(unsigned char *buf, unsigned font_height)
{
  unsigned char seq2, seq4, gc4, gc5, gc6;
  unsigned i;

  // save registers set_plane() modifies GC 4 and SEQ 2, so save them as well
  outportb(VGA_SEQ_INDEX, 2);
  seq2 = inportb(VGA_SEQ_DATA);

  outportb(VGA_SEQ_INDEX, 4);
  seq4 = inportb(VGA_SEQ_DATA);
  // turn off even-odd addressing (set flat addressing)
  // assume: chain-4 addressing already off
  outportb(VGA_SEQ_DATA, seq4 | 0x04);

  outportb(VGA_GC_INDEX, 4);
  gc4 = inportb(VGA_GC_DATA);

  outportb(VGA_GC_INDEX, 5);
  gc5 = inportb(VGA_GC_DATA);
  // turn off even-odd addressing
  outportb(VGA_GC_DATA, gc5 & ~0x10);

  outportb(VGA_GC_INDEX, 6);
  gc6 = inportb(VGA_GC_DATA);
  // turn off even-odd addressing
  outportb(VGA_GC_DATA, gc6 & ~0x02);
  // write font to plane P4
  set_plane(2);
  // write font 0
  for(i = 0; i < 256; i++)
  {
    vmemwr(16384u * 0 + i * 32, buf, font_height);
    buf += font_height;
  }
#if 0
  // write font 1
  for(i = 0; i < 256; i++)
  {
    vmemwr(16384u * 1 + i * 32, buf, font_height);
    buf += font_height;
  }
#endif
  // restore registers
  outportb(VGA_SEQ_INDEX, 2);
  outportb(VGA_SEQ_DATA, seq2);
  outportb(VGA_SEQ_INDEX, 4);
  outportb(VGA_SEQ_DATA, seq4);
  outportb(VGA_GC_INDEX, 4);
  outportb(VGA_GC_DATA, gc4);
  outportb(VGA_GC_INDEX, 5);
  outportb(VGA_GC_DATA, gc5);
  outportb(VGA_GC_INDEX, 6);
  outportb(VGA_GC_DATA, gc6);
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel1(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes;
  unsigned off, mask;

  c = (c & 1) * 0xFF;
  wd_in_bytes = width / 8;
  off = wd_in_bytes * y + x / 8;
  x = (x & 7) * 1;
  mask = 0x80 >> x;
  fb[off] = (fb[off] & ~mask) | (c & mask);
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel2(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes, off, mask;

  c = (c & 3) * 0x55;
  wd_in_bytes = width / 4;
  off = wd_in_bytes * y + x / 4;
  x = (x & 3) * 2;
  mask = 0xC0 >> x;
  fb[off] =  (fb[off] & ~mask) | (c & mask);
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel4p(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes, off, mask, p, pmask;

  wd_in_bytes = width / 8;
  off = wd_in_bytes * y + x / 8;
  x = (x & 7) * 1;
  mask = 0x80 >> x;
  pmask = 1;
  for(p = 0; p < 4; p++)
  {
    set_plane(p);
    if(pmask & c)
      fb[off] = fb[off] | mask;
    else
      fb[off] = fb[off] & ~mask;
    pmask <<= 1;
  }
}
*/
// -------------------------------------------------------------------------- //
//! Place a pixel on the screen, at (x,y) using color c.
static void write_pixel8(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes;
  unsigned off;

  wd_in_bytes = width;
  off = wd_in_bytes * y + x;
  fb[off] = (uint8_t)c;
}
// -------------------------------------------------------------------------- //
/*
static void write_pixel8x(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes;
  unsigned off;

  wd_in_bytes = width / 4;
  off = wd_in_bytes * y + x / 4;
  set_plane(x & 3);
  fb[off] = (uint8_t)c;
}
*/
// -------------------------------------------------------------------------- //
/*
static void draw_x(void)
{
  unsigned x, y;

  // clear screen
  for(y = 0; y < g_ht; y++)
    for(x = 0; x < width; x++)
      vga_write_pixel(x, y, 0);
  // draw 2-color X
  for(y = 0; y < g_ht; y++)
  {
    vga_write_pixel((width - g_ht) / 2 + y, y, 1);
    vga_write_pixel((g_ht + width) / 2 - y, y, 2);
  }
}
*/
// -------------------------------------------------------------------------- //
//! Set all pixels on the screen to the same given color.A.K.A. clear the screen
void vga_clear(const uint8_t c)
{
  uint32_t x, y;
  for( y = 0; y < height; y++ )
  {
    for( x = 0; x < width; x++ )
    {
      fb[y*width+x] = c;
    }
  }
  vga_flip();
}
// -------------------------------------------------------------------------- //
/*
uint32_t vga_width()
{
  return width;
}
uint32_t vga_height()
{
  return height;
}
*/
// -------------------------------------------------------------------------- //
/**
 * Draw a line on the screen, between the two points given, with the given color
 */
void vga_line(const unsigned int x0, const unsigned int y0,
  const unsigned int x1, const unsigned int y1, const uint8_t c)
{
    int dx, dy, p, x, y;

    dx=x1-x0;
    dy=y1-y0;

    x=x0;
    y=y0;

    p=2*dy-dx;

    while(x<x1)
    {
        if(p>=0)
        {
            fb[y*width+x] = c;
            y=y+1;
            p=p+2*dy-2*dx;
        }
        else
        {
            fb[y*width+x] = c;
            p=p+2*dy;
        }
        x=x+1;
    }
}
// -------------------------------------------------------------------------- //
/**
 * Draw filled recrangle, given two points and a fill color
 */
void drawrect_fill(const uint32_t left, const uint32_t top,
  const uint32_t right, const uint32_t bottom, const uint8_t color)
{
  uint32_t top_offset, bottom_offset, i, temp, w;
  uint32_t new_left = left;
  uint32_t new_top = top;
  uint32_t new_right = right;
  uint32_t new_bottom = bottom;

  if (new_top>new_bottom)
  {
    temp = new_top;
    new_top = new_bottom;
    new_bottom = temp;
  }
  if (new_left>new_right)
  {
    temp = new_left;
    new_left = new_right;
    new_right = temp;
  }

  top_offset = (new_top<<8) + (new_top<<6) + new_left;
  bottom_offset = (new_bottom<<8) + (new_bottom<<6) + new_left;
  w = new_right - new_left + 1;

  for(i=top_offset; i<=bottom_offset; i+=width)
  {
    memset(&fb[i],color,w-1);
  }
}
// -------------------------------------------------------------------------- //
void vga_set_graphic()
{
  write_regs(module_video_mode_320x200x256);
//  write_regs(g_640x480x16);
//  write_regs(module_video_mode_720x480x16);
  vga_write_pixel = write_pixel8;// 4-bit = 16 colors
  // fb = (uint8_t*)vga_get_fb();
  fb = (uint8_t*)kmalloc(width * height * ( depth >> 3 ));

  width  = 320;  height = 200;  depth  = 8;
//  width  = 640;  height = 480;  depth  = 8;

  vga_set_RRRGGGBB();

  // if you don't flip first, you'll get static, from random data as pixels
  vga_flip();

  // clean screen
//  vga_clear(RGB_TO_332(0xA0,0xA0,0xA0)); // already does flip

  // one pixel
  write_pixel8(1, 30, RGB_TO_332(0xFF,0x0,0x0));

  // line
  vga_line(5, 5, 20, 20, RGB_TO_332(0xA0,0xA0,0xA0));

  // one rectangle - filled
  drawrect_fill(1, 1, 5, 5, RGB_TO_332(0x0A,0xA0,0xA0));

  // one character
  {
    uint8_t x = 20;
    uint8_t color = RGB_TO_332(0xA0,0xA0,0xA0);
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'G'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'R'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'A'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'p'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'h'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'i'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 'c'); x+= 20;
    vga_putchar(x, 20, color, RGB_TO_332(0x13,0x13,0x13), 's');
  }

  // flip again to display what you drew
  vga_flip();

  // wait keypress and return to text mode
//  module_keyboard_wait_keypress();
//  write_regs(g_80x25_text); // not working as it should
//  module_terminal_global_print_c_string("Back to text mode!\n");

//  uint16_t y;
//  for(y=0;y<200;y++)
//    drawline(0,y,width,y,y & 0xFF);
//  drawrect_fill(0,0,width,10,20);
}
// -------------------------------------------------------------------------- //
//! Copy from RAM video buffer to video card buffer - so that the pixels appear.
void vga_flip()
{
  memcpy(fb,(uint8_t*)vga_get_fb(), width * height * ( depth >> 3 ));
  memset(fb, 0, width * height * ( depth >> 3 )); // clear buffer
}
// -------------------------------------------------------------------------- //
void vga_set_RRRGGGBB()
{
  uint32_t i;

  for( i=0; i < 256; i++)
  {
    outportb(VGA_DAC_WRITE_INDEX, i);
    // use high values for RGB that fits on channel's bits.
    // note that VGA has 18-bit color on pallete (6-bit per channel)
    // so each channel will be right-shifted >> 2
    outportb(VGA_DAC_DATA, (i & 0xE0) >> 2 );
    outportb(VGA_DAC_DATA, (i & 0x1C) << 1 ); // (i & 0x1C) << 3) >> 2
    outportb(VGA_DAC_DATA, (i & 0x3) << 4 ); // ((i & 0x3) << 6) >> 2
  }
}
// -------------------------------------------------------------------------- //
//! Print/Draw one character on screen, pixel by pixel based on fonts
void vga_putchar(const uint32_t x, const uint32_t y, const uint8_t fgcolor,
  const uint8_t bgcolor, const char c)
{
  uint8_t i, j;
  for(i = 0; i < 8; i++)
  {
    for(j = 0; j < 8; j++)
    {
      vga_write_pixel(x+i, y+j,
        ((module_video_font_8x8_basic[c & 0x7F][j] >> i ) & 1)?fgcolor:bgcolor);
    }
  }
}
// -------------------------------------------------------------------------- //
void module_video_test(module_heap_heap_bm *h)
{
  module_video_heap = h;
  vga_set_graphic();
}
// -------------------------------------------------------------------------- //
