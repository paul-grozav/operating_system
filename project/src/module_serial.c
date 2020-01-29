// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include <stdint.h> // uintX
#include "module_serial.h"
// -------------------------------------------------------------------------- //
#define PORT 0x3f8 // COM1
// -------------------------------------------------------------------------- //
uint8_t inb(const uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
// -------------------------------------------------------------------------- //
void outb(const uint16_t port, const uint8_t value)
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
void init_serial()
{
  outb(PORT + 1, 0x00); // Disable all interrupts
  outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
  outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
  outb(PORT + 1, 0x00); //                  (hi byte)
  outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
  outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}
// -------------------------------------------------------------------------- //
// Sending data
int is_transmit_empty()
{
  return inb(PORT + 5) & 0x20;
}
// -------------------------------------------------------------------------- //
void write_serial(const char a)
{
  while (is_transmit_empty() == 0);
  outb(PORT, a);
}
// -------------------------------------------------------------------------- //
// Receiving data
int serial_received()
{
  return inb(PORT + 5) & 1;
}
// -------------------------------------------------------------------------- //
char read_serial()
{
  while (serial_received() == 0);
  return inb(PORT);
}
// -------------------------------------------------------------------------- //
void module_serial_print_c_string(const char* str)
{
  // Keep placing characters until we hit the null-terminating character ('\0')
  for (size_t i = 0; str[i] != '\0'; i ++)
  {
    write_serial(str[i]);
  }
}
// -------------------------------------------------------------------------- //
void module_serial_test()
{
  init_serial();
  module_serial_print_c_string(
    "This is just a test for serial communications.\n");

  // notify the other end that the serial communication can now begin
  write_serial(0);
  if(serial_received() == 1)
  {
    module_serial_print_c_string("Serial has data.\n");
  }
  else
  {
    module_serial_print_c_string("Serial has no data.\n");
  }
//  char c = 0;
//  c = read_serial(); write_serial(c);
}
// -------------------------------------------------------------------------- //

