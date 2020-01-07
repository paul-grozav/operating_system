// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module.h"
void module(volatile uint16_t* b, size_t idx)
{
  uint8_t term_color = 0x0F;
  b[idx++] = ((uint16_t)term_color << 8) | 'M';
  b[idx++] = ((uint16_t)term_color << 8) | 'O';
  b[idx++] = ((uint16_t)term_color << 8) | 'D';
  b[idx++] = ((uint16_t)term_color << 8) | 'U';
  b[idx++] = ((uint16_t)term_color << 8) | 'L';
  b[idx++] = ((uint16_t)term_color << 8) | 'E';
}
// -------------------------------------------------------------------------- //

