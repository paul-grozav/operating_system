// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include "module_kernel.h"
// -------------------------------------------------------------------------- //
// In/Out bytes
// -------------------------------------------------------------------------- //
void module_kernel_out_byte(const uint16_t port, const uint8_t value)
{
  // There's an outb %al, $imm8  encoding, for compile-time constant port
  // numbers that fit in 8b. (N constraint).
  // Wider immediate constants would be truncated at assemble-time
  // (e.g. "i" constraint).
  // The  outb  %al, %dx  encoding is the only option for all other cases.
  // %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we
  // had the port number a wider C type
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
// -------------------------------------------------------------------------- //
uint8_t module_kernel_in_byte(const uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
// -------------------------------------------------------------------------- //
// Memory stuff
// -------------------------------------------------------------------------- //
void module_kernel_memset(void *start, const char value, const size_t length)
{
  char *buffer = (char*)start;
  for(size_t i=0; i<length; i++)
  {
    buffer[i] = value;
  }
}
// -------------------------------------------------------------------------- //
