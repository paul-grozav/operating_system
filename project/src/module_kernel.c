// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include "module_kernel.h"
// -------------------------------------------------------------------------- //
// In/Out bytes
// -------------------------------------------------------------------------- //
void module_kernel_out_8(const uint16_t port, const uint8_t value)
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
uint8_t module_kernel_in_8(const uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
// -------------------------------------------------------------------------- //
void module_kernel_out_16(const uint16_t port, const uint16_t value)
{
  __asm__ __volatile__("outw %%ax,%%dx"::"d" (port), "a" (value));
}
// -------------------------------------------------------------------------- //
uint16_t module_kernel_in_16(const uint16_t port)
{
  uint16_t ret;
  __asm__ __volatile__("inw %%dx,%%ax":"=a" (ret):"d"(port));
  return ret;
}
// -------------------------------------------------------------------------- //
void module_kernel_out_32(const uint16_t port, const uint32_t value)
{
  __asm__ __volatile__("outl %%eax,%%dx"::"d" (port), "a" (value));
}
// -------------------------------------------------------------------------- //
uint32_t module_kernel_in_32(const uint16_t port)
{
  uint32_t ret;
  __asm__ __volatile__("inl %%dx,%%eax":"=a" (ret):"d"(port));
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
void module_kernel_memcpy(const void * const source, void* destination,
  const size_t size)
{
  for(size_t i=0; i<size; i++)
  {
    ((uint8_t*)destination)[i] = ((uint8_t*)source)[i];
  }
}
// -------------------------------------------------------------------------- //
int8_t module_kernel_memcmp(const void * const a,
  const void * const b, const size_t size)
{
  for(size_t i=0; i<size; i++)
  {
    if( *(((const uint8_t*)(a)) + i)  !=  *(((const uint8_t*)(b)) + i) )
    {
      if( *(((const uint8_t*)(a)) + i)  <  *(((const uint8_t*)(b)) + i) )
      {
        return -1;
      }
      else
      {
        return 1;
      }
    }
  }
  return 0;
}
// -------------------------------------------------------------------------- //
