// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_video_mode.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// VGA REGISTER DUMPS FOR VARIOUS TEXT MODES
// -------------------------------------------------------------------------- //
unsigned char g_40x25_text[] =
{
// MISC
  0x67,
// SEQ
  0x03, 0x08, 0x03, 0x00, 0x02,
// CRTC
  0x2D, 0x27, 0x28, 0x90, 0x2B, 0xA0, 0xBF, 0x1F,
  0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0xA0,
  0x9C, 0x8E, 0x8F, 0x14, 0x1F, 0x96, 0xB9, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x0C, 0x00, 0x0F, 0x08, 0x00,
};
// -------------------------------------------------------------------------- //
unsigned char g_40x50_text[] =
{
// MISC
  0x67,
// SEQ
  0x03, 0x08, 0x03, 0x00, 0x02,
// CRTC
  0x2D, 0x27, 0x28, 0x90, 0x2B, 0xA0, 0xBF, 0x1F,
  0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x04, 0x60,
  0x9C, 0x8E, 0x8F, 0x14, 0x1F, 0x96, 0xB9, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x0C, 0x00, 0x0F, 0x08, 0x00,
};
// -------------------------------------------------------------------------- //
unsigned char g_80x25_text[] =
{
// MISC
  0x67,
// SEQ
  0x03, 0x00, 0x03, 0x00, 0x02,
// CRTC
  0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
  0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
  0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x0C, 0x00, 0x0F, 0x08, 0x00
};
// -------------------------------------------------------------------------- //
unsigned char g_80x50_text[] =
{
// MISC
  0x67,
// SEQ
  0x03, 0x00, 0x03, 0x00, 0x02,
// CRTC
  0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
  0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x01, 0x40,
  0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x0C, 0x00, 0x0F, 0x08, 0x00,
};
// -------------------------------------------------------------------------- //
unsigned char g_90x30_text[] =
{
// MISC
  0xE7,
// SEQ
  0x03, 0x01, 0x03, 0x00, 0x02,
// CRTC
  0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
  0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0xEA, 0x0C, 0xDF, 0x2D, 0x10, 0xE8, 0x05, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x0C, 0x00, 0x0F, 0x08, 0x00,
};
// -------------------------------------------------------------------------- //
unsigned char g_90x60_text[] =
{
// MISC
  0xE7,
// SEQ
  0x03, 0x01, 0x03, 0x00, 0x02,
// CRTC
  0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
  0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
  0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x0C, 0x00, 0x0F, 0x08, 0x00,
};
// -------------------------------------------------------------------------- //
// VGA REGISTER DUMPS FOR VARIOUS GRAPHICS MODES
// -------------------------------------------------------------------------- //
unsigned char g_640x480x2[] =
{
// MISC
  0xE3,
// SEQ
  0x03, 0x01, 0x0F, 0x00, 0x06,
// CRTC
  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x01, 0x00, 0x0F, 0x00, 0x00
};
// -------------------------------------------------------------------------- //
// NOTE: the mode described by g_320x200x4[]
// is different from BIOS mode 05h in two ways:
// - Framebuffer is at A000:0000 instead of B800:0000
// - Framebuffer is linear (no screwy line-by-line CGA addressing)
// -------------------------------------------------------------------------- //
unsigned char g_320x200x4[] =
{
// MISC
  0x63,
// SEQ
  0x03, 0x09, 0x03, 0x00, 0x02,
// CRTC
  0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80, 0xBF, 0x1F,
  0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x9C, 0x0E, 0x8F, 0x14, 0x00, 0x96, 0xB9, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x02, 0x00,
  0xFF,
// AC
  0x00, 0x13, 0x15, 0x17, 0x02, 0x04, 0x06, 0x07,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x01, 0x00, 0x03, 0x00, 0x00
};
// -------------------------------------------------------------------------- //
unsigned char g_640x480x16[] =
{
// MISC
  0xE3,
// SEQ
  0x03, 0x01, 0x08, 0x00, 0x06,
// CRTC
  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x01, 0x00, 0x0F, 0x00, 0x00
};
// -------------------------------------------------------------------------- //
unsigned char module_video_mode_720x480x16[] =
{
// MISC
  0xE7,
// SEQ
  0x03, 0x01, 0x08, 0x00, 0x06,
// CRTC
  0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
  0x00, 0x40, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
  0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xE3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x01, 0x00, 0x0F, 0x00, 0x00,
};
// -------------------------------------------------------------------------- //
unsigned char module_video_mode_320x200x256[] =
{
// MISC
  0x63,
// SEQ
  0x03, 0x01, 0x0F, 0x00, 0x0E,
// CRTC
  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
  0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x9C, 0x0E, 0x8F, 0x28,  0x40, 0x96, 0xB9, 0xA3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x41, 0x00, 0x0F, 0x00,  0x00
};
// -------------------------------------------------------------------------- //
unsigned char g_320x200x256_modex[] =
{
// MISC
  0x63,
// SEQ
  0x03, 0x01, 0x0F, 0x00, 0x06,
// CRTC
  0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
  0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x9C, 0x0E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
  0xFF,
// GC
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
  0xFF,
// AC
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x41, 0x00, 0x0F, 0x00, 0x00
};
// -------------------------------------------------------------------------- //
// g_360x480x256_modex - to do
// -------------------------------------------------------------------------- //

