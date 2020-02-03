// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include <stdint.h> // uintX
#include "module_kernel.h" // in/out byte
#include "module_serial.h"
// -------------------------------------------------------------------------- //
#define PORT 0x3f8 // COM1
// -------------------------------------------------------------------------- //
void init_serial()
{
  // Disable all interrupts
  module_kernel_out_8(PORT + 1, 0x00);
  // Enable DLAB (set baud rate divisor)
  module_kernel_out_8(PORT + 3, 0x80);
  // Set divisor to 3 (lo byte) 38400 baud
  module_kernel_out_8(PORT + 0, 0x03);
  //                  (hi byte)
  module_kernel_out_8(PORT + 1, 0x00);
  // 8 bits, no parity, one stop bit
  module_kernel_out_8(PORT + 3, 0x03);
  // Enable FIFO, clear them, with 14-byte threshold
  module_kernel_out_8(PORT + 2, 0xC7);
  // IRQs enabled, RTS/DSR set
  module_kernel_out_8(PORT + 4, 0x0B);
}
// -------------------------------------------------------------------------- //
// Sending data
int is_transmit_empty()
{
  return module_kernel_in_8(PORT + 5) & 0x20;
}
// -------------------------------------------------------------------------- //
void write_serial(const char a)
{
  while (is_transmit_empty() == 0);
  module_kernel_out_8(PORT, a);
}
// -------------------------------------------------------------------------- //
// Receiving data
int serial_received()
{
  return module_kernel_in_8(PORT + 5) & 1;
}
// -------------------------------------------------------------------------- //
char read_serial()
{
  while (serial_received() == 0);
  return module_kernel_in_8(PORT);
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

