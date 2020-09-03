// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_kernel.h"
#include "module_heap.h"
//#include "module_terminal.h"
//#include "module_keyboard.h"
#include "module_video.h"
#include "module_video_mode.h"
#include "module_video_font.h"
// -------------------------------------------------------------------------- //
#define RGB_TO_332(R,G,B)  ((R & 0xE0) | ((G  >> 3 ) & 0x1C) | (B >> 6))
#define VGA_AC_INDEX          0x3C0
#define VGA_AC_WRITE          0x3C0
#define VGA_AC_READ           0x3C1
#define VGA_MISC_WRITE        0x3C2
#define VGA_SEQ_INDEX         0x3C4
#define VGA_SEQ_DATA          0x3C5
//#define  VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX   0x3C8
#define VGA_DAC_DATA          0x3C9
#define VGA_MISC_READ         0x3CC
#define VGA_GC_INDEX          0x3CE
#define VGA_GC_DATA           0x3CF
// COLOR emulation    MONO emulation
#define VGA_CRTC_INDEX        0x3D4 // 0x3B4
#define VGA_CRTC_DATA         0x3D5 // 0x3B5
#define VGA_INSTAT_READ       0x3DA
#define VGA_NUM_SEQ_REGS       5
#define VGA_NUM_CRTC_REGS     25
#define VGA_NUM_GC_REGS        9
#define VGA_NUM_AC_REGS       21
//#define VGA_NUM_REGS          (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS +
//  VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
// -------------------------------------------------------------------------- //
uint32_t module_video_width, module_video_height, module_video_depth;
uint8_t* module_video_fb;
// -------------------------------------------------------------------------- //
//! Used to switch between graphics and text modes.
void module_video_write_regs(unsigned char *regs)
{
  unsigned i;

  // write MISCELLANEOUS reg
  module_kernel_out_8(VGA_MISC_WRITE, *regs);
  regs++;
  // write SEQUENCER regs
  for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
  {
    module_kernel_out_8(VGA_SEQ_INDEX, i);
    module_kernel_out_8(VGA_SEQ_DATA, *regs);
    regs++;
  }
  // unlock CRTC registers
  module_kernel_out_8(VGA_CRTC_INDEX, 0x03);
  module_kernel_out_8(VGA_CRTC_DATA, module_kernel_in_8(VGA_CRTC_DATA) | 0x80);
  module_kernel_out_8(VGA_CRTC_INDEX, 0x11);
  module_kernel_out_8(VGA_CRTC_DATA, module_kernel_in_8(VGA_CRTC_DATA) & ~0x80);
  // make sure they remain unlocked
  regs[0x03] |= 0x80;
  regs[0x11] &= ~0x80;
  // write CRTC regs
  for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
  {
    module_kernel_out_8(VGA_CRTC_INDEX, i);
    module_kernel_out_8(VGA_CRTC_DATA, *regs);
    regs++;
  }
  // write GRAPHICS CONTROLLER regs
  for(i = 0; i < VGA_NUM_GC_REGS; i++)
  {
    module_kernel_out_8(VGA_GC_INDEX, i);
    module_kernel_out_8(VGA_GC_DATA, *regs);
    regs++;
  }
  // write ATTRIBUTE CONTROLLER regs
  for(i = 0; i < VGA_NUM_AC_REGS; i++)
  {
    (void)module_kernel_in_8(VGA_INSTAT_READ);
    module_kernel_out_8(VGA_AC_INDEX, i);
    module_kernel_out_8(VGA_AC_WRITE, *regs);
    regs++;
  }
  // lock 16-color palette and unblank display
  (void)module_kernel_in_8(VGA_INSTAT_READ);
  module_kernel_out_8(VGA_AC_INDEX, 0x20);
}
// -------------------------------------------------------------------------- //
//! VGA address where we should write to draw on the screen
unsigned module_video_vga_get_fb()
{
  unsigned seg;
  module_kernel_out_8(VGA_GC_INDEX, 6);
  seg = module_kernel_in_8(VGA_GC_DATA);
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
//! Place a pixel on the screen, at (x,y) using color c.
void module_video_write_pixel8(const unsigned x, const unsigned y,
  const unsigned char c)
{
  unsigned wd_in_bytes;
  unsigned off;

  wd_in_bytes = module_video_width;
  off = wd_in_bytes * y + x;
  module_video_fb[off] = (uint8_t)c;
}
// -------------------------------------------------------------------------- //
/**
 * Draw a line on the screen, between the two points given, with the given color
 */
void module_video_vga_line(const unsigned int x0, const unsigned int y0,
  const unsigned int x1, const unsigned int y1, const uint8_t c)
{
  unsigned int x = 0;
  int dx, dy, p, y;

  dx = x1 - x0;
  dy = y1 - y0;

  x = x0;
  y = y0;

  p = 2*dy - dx;

  while(x<x1)
  {
    if(p>=0)
    {
      module_video_fb[y*module_video_width+x] = c;
      y = y + 1;
      p = p + 2*dy - 2*dx;
    }
    else
    {
      module_video_fb[y*module_video_width+x] = c;
      p = p + 2*dy;
    }
    x = x + 1;
  }
}
// -------------------------------------------------------------------------- //
/**
 * Draw filled recrangle, given two points and a fill color
 */
void module_video_drawrect_fill(const uint32_t left, const uint32_t top,
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

  for(i=top_offset; i<=bottom_offset; i+=module_video_width)
  {
    module_kernel_memset(&module_video_fb[i],color,w-1);
  }
}
// -------------------------------------------------------------------------- //
//! Copy from RAM video buffer to video card buffer - so that the pixels appear.
void module_video_vga_flip()
{
  // copy from RAM to video card buffer
  module_kernel_memcpy(module_video_fb, (uint8_t*)module_video_vga_get_fb(),
    module_video_width * module_video_height * ( module_video_depth >> 3 ));
  // clear buffer from RAM
  module_kernel_memset(module_video_fb, 0,
    module_video_width * module_video_height * ( module_video_depth >> 3 ));
}
// -------------------------------------------------------------------------- //
void module_video_vga_set_RRRGGGBB()
{
  for(uint32_t i=0; i < 256; i++)
  {
    module_kernel_out_8(VGA_DAC_WRITE_INDEX, i);
    // use high values for RGB that fits on channel's bits.
    // note that VGA has 18-bit color on pallete (6-bit per channel)
    // so each channel will be right-shifted >> 2
    module_kernel_out_8(VGA_DAC_DATA, (i & 0xE0) >> 2 );
    module_kernel_out_8(VGA_DAC_DATA, (i & 0x1C) << 1 ); //(i & 0x1C) << 3) >> 2
    module_kernel_out_8(VGA_DAC_DATA, (i & 0x3) << 4 ); // ((i & 0x3) << 6) >> 2
  }
}
// -------------------------------------------------------------------------- //
//! Print/Draw one character on screen, pixel by pixel based on fonts
void module_video_vga_putchar(const uint32_t x, const uint32_t y,
  const uint8_t fgcolor, const uint8_t bgcolor, const char c)
{
  uint8_t i, j;
  for(i = 0; i < 8; i++)
  {
    for(j = 0; j < 8; j++)
    {
      //vga_write_pixel
      module_video_write_pixel8(x+i, y+j,
        ((module_video_font_8x8_basic[c & 0x7F][j] >> i ) & 1)?fgcolor:bgcolor);
    }
  }
}
// -------------------------------------------------------------------------- //
void module_video_test(module_heap_heap_bm *h)
{
  module_video_write_regs(module_video_mode_320x200x256);
//  module_video_write_regs(g_640x480x16);
//  module_video_write_regs(module_video_mode_720x480x16);
//  vga_write_pixel = module_video_write_pixel8;// 4-bit = 16 colors
  // module_video_fb = (uint8_t*)module_video_vga_get_fb();
  module_video_fb = (uint8_t*)module_heap_alloc(h,
    module_video_width * module_video_height * ( module_video_depth >> 3 ));

  module_video_width = 320;
  module_video_height = 200;
  module_video_depth = 8;

  module_video_vga_set_RRRGGGBB();

  // if you don't flip first, you'll get static, from random data as pixels
  module_video_vga_flip();

  // clean screen
//  module_video_vga_clear(RGB_TO_332(0xA0,0xA0,0xA0)); // already does flip

  // one pixel
  module_video_write_pixel8(1, 30, RGB_TO_332(0xFF,0x0,0x0));

  // line
  module_video_vga_line(5, 5, 20, 20, RGB_TO_332(0xA0,0xA0,0xA0));

  // one rectangle - filled
  module_video_drawrect_fill(1, 1, 5, 5, RGB_TO_332(0x0A,0xA0,0xA0));

  // draw/print text
  {
    uint8_t i=0;
    uint8_t fgcolor = RGB_TO_332(0xA0,0xA0,0xA0);
    uint8_t bgcolor = RGB_TO_332(0x13,0x13,0x13);
    char *txt = "GRAphics\n";
    while(txt[i] != '\0')
    {
      module_video_vga_putchar(20*(i+1), 20, fgcolor, bgcolor, txt[i]);
      i += 1;
    }
  }

  // flip again to display what you drew
  module_video_vga_flip();

  // wait keypress and return to text mode
//  module_keyboard_wait_keypress();
//  module_video_write_regs(g_80x25_text); // not working as it should
//  module_terminal_global_print_c_string("Back to text mode!\n");

//  uint16_t y;
//  for(y=0;y<200;y++)
//    drawline(0,y,module_video_width,y,y & 0xFF);
//  module_video_drawrect_fill(0,0,module_video_width,10,20);
}
// -------------------------------------------------------------------------- //
// ----------------------- MIGHT BE USEFUL LATER ---------------------------- //
// -------------------------------------------------------------------------- //
//void vga_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
//  uint8_t color);

//void (*putpixel)(unsigned x, unsigned y, unsigned c);
//void (*vga_write_pixel)(unsigned x, unsigned y, unsigned char c);

//static unsigned g_wd, g_ht;

/*
uint32_t vga_width();
uint32_t vga_height();

uint32_t vga_width()
{
  return module_video_width;
}
uint32_t vga_height()
{
  return module_video_height;
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel8x(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes;
  unsigned off;

  wd_in_bytes = module_video_width / 4;
  off = wd_in_bytes * y + x / 4;
  module_video_set_plane(x & 3);
  module_video_fb[off] = (uint8_t)c;
}
*/
// -------------------------------------------------------------------------- //
/*
static void draw_x(void)
{
  unsigned x, y;

  // clear screen
  for(y = 0; y < g_ht; y++)
    for(x = 0; x < module_video_width; x++)
      vga_write_pixel(x, y, 0);
  // draw 2-color X
  for(y = 0; y < g_ht; y++)
  {
    vga_write_pixel((module_video_width - g_ht) / 2 + y, y, 1);
    vga_write_pixel((g_ht + module_video_width) / 2 - y, y, 2);
  }
}
*/
// -------------------------------------------------------------------------- //
/*
// -------------------------------------------------------------------------- //
// write font to plane P4 (assuming planes are named P1, P2, P4, P8)
// -------------------------------------------------------------------------- //
static void write_font(unsigned char *buf, unsigned font_height)
{
  unsigned char seq2, seq4, gc4, gc5, gc6;
  unsigned i;

  // save registers module_video_set_plane() modifies GC 4 and SEQ 2, so save
  // them as well
  module_kernel_out_8(VGA_SEQ_INDEX, 2);
  seq2 = module_kernel_in_8(VGA_SEQ_DATA);

  module_kernel_out_8(VGA_SEQ_INDEX, 4);
  seq4 = module_kernel_in_8(VGA_SEQ_DATA);
  // turn off even-odd addressing (set flat addressing)
  // assume: chain-4 addressing already off
  module_kernel_out_8(VGA_SEQ_DATA, seq4 | 0x04);

  module_kernel_out_8(VGA_GC_INDEX, 4);
  gc4 = module_kernel_in_8(VGA_GC_DATA);

  module_kernel_out_8(VGA_GC_INDEX, 5);
  gc5 = module_kernel_in_8(VGA_GC_DATA);
  // turn off even-odd addressing
  module_kernel_out_8(VGA_GC_DATA, gc5 & ~0x10);

  module_kernel_out_8(VGA_GC_INDEX, 6);
  gc6 = module_kernel_in_8(VGA_GC_DATA);
  // turn off even-odd addressing
  module_kernel_out_8(VGA_GC_DATA, gc6 & ~0x02);
  // write font to plane P4
  module_video_set_plane(2);
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
  module_kernel_out_8(VGA_SEQ_INDEX, 2);
  module_kernel_out_8(VGA_SEQ_DATA, seq2);
  module_kernel_out_8(VGA_SEQ_INDEX, 4);
  module_kernel_out_8(VGA_SEQ_DATA, seq4);
  module_kernel_out_8(VGA_GC_INDEX, 4);
  module_kernel_out_8(VGA_GC_DATA, gc4);
  module_kernel_out_8(VGA_GC_INDEX, 5);
  module_kernel_out_8(VGA_GC_DATA, gc5);
  module_kernel_out_8(VGA_GC_INDEX, 6);
  module_kernel_out_8(VGA_GC_DATA, gc6);
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel1(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes;
  unsigned off, mask;

  c = (c & 1) * 0xFF;
  wd_in_bytes = module_video_width / 8;
  off = wd_in_bytes * y + x / 8;
  x = (x & 7) * 1;
  mask = 0x80 >> x;
  module_video_fb[off] = (module_video_fb[off] & ~mask) | (c & mask);
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel2(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes, off, mask;

  c = (c & 3) * 0x55;
  wd_in_bytes = module_video_width / 4;
  off = wd_in_bytes * y + x / 4;
  x = (x & 3) * 2;
  mask = 0xC0 >> x;
  module_video_fb[off] =  (module_video_fb[off] & ~mask) | (c & mask);
}
*/
// -------------------------------------------------------------------------- //
/*
static void write_pixel4p(unsigned x, unsigned y, unsigned char c)
{
  unsigned wd_in_bytes, off, mask, p, pmask;

  wd_in_bytes = module_video_width / 8;
  off = wd_in_bytes * y + x / 8;
  x = (x & 7) * 1;
  mask = 0x80 >> x;
  pmask = 1;
  for(p = 0; p < 4; p++)
  {
    module_video_set_plane(p);
    if(pmask & c)
      module_video_fb[off] = module_video_fb[off] | mask;
    else
      module_video_fb[off] = module_video_fb[off] & ~mask;
    pmask <<= 1;
  }
}
*/
// -------------------------------------------------------------------------- //
/*
void read_regs(unsigned char *regs)
{
  unsigned i;

// read MISCELLANEOUS reg
  *regs = module_kernel_in_8(VGA_MISC_READ);
  regs++;
// read SEQUENCER regs
  for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
  {
    module_kernel_out_8(VGA_SEQ_INDEX, i);
    *regs = module_kernel_in_8(VGA_SEQ_DATA);
    regs++;
  }
// read CRTC regs
  for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
  {
    module_kernel_out_8(VGA_CRTC_INDEX, i);
    *regs = module_kernel_in_8(VGA_CRTC_DATA);
    regs++;
  }
// read GRAPHICS CONTROLLER regs
  for(i = 0; i < VGA_NUM_GC_REGS; i++)
  {
    module_kernel_out_8(VGA_GC_INDEX, i);
    *regs = module_kernel_in_8(VGA_GC_DATA);
    regs++;
  }
// read ATTRIBUTE CONTROLLER regs
  for(i = 0; i < VGA_NUM_AC_REGS; i++)
  {
    (void)module_kernel_in_8(VGA_INSTAT_READ);
    module_kernel_out_8(VGA_AC_INDEX, i);
    *regs = module_kernel_in_8(VGA_AC_READ);
    regs++;
  }
// lock 16-color palette and unblank display
  (void)module_kernel_in_8(VGA_INSTAT_READ);
  module_kernel_out_8(VGA_AC_INDEX, 0x20);
}
*/
// -------------------------------------------------------------------------- //
/*
static void module_video_set_plane(unsigned p)
{
  unsigned char pmask;
  p &= 3;
  pmask = 1 << p;
  // set read plane
  module_kernel_out_8(VGA_GC_INDEX, 4);
  module_kernel_out_8(VGA_GC_DATA, p);
  // set write plane
  module_kernel_out_8(VGA_SEQ_INDEX, 2);
  module_kernel_out_8(VGA_SEQ_DATA, pmask);
}
*/
// -------------------------------------------------------------------------- //
/*
static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count)
{
  unsigned i = 0;
  unsigned char *lfb = (unsigned char*)module_video_vga_get_fb();
  for (i = 0; i < count; i++)
  {
    lfb[dst_off+i] = src[i];
  }
}
*/
// -------------------------------------------------------------------------- //
//! Set all pixels on the screen to the same given color.A.K.A. clear the screen
/*
//void module_video_vga_clear(uint8_t c);
void module_video_vga_clear(const uint8_t c)
{
  uint32_t x, y;
  for( y = 0; y < module_video_height; y++ )
  {
    for( x = 0; x < module_video_width; x++ )
    {
      module_video_fb[y*module_video_width+x] = c;
    }
  }
  module_video_vga_flip();
}
*/
// -------------------------------------------------------------------------- //

