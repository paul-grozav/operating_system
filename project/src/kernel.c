// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// GCC provides these header files automatically
// They give us access to useful things like fixed-width types
#include <stdint.h>
#include "module_terminal.h"
#include "module_serial.h"
// -------------------------------------------------------------------------- //
// First, let's do some basic checks to make sure we are using our x86-elf
// cross-compiler correctly
#if defined(__linux__)
  #error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
  #error "This code must be compiled with an x86-elf compiler"
#endif
// -------------------------------------------------------------------------- //
// This is our kernel's main function
void kernel_main()
{
  // We're here! Let's initiate the terminal and display a message to show we
  // got here.

  module_terminal_vga t = module_terminal_vga_create();
  // Initiate terminal
  module_terminal_init(&t);

  // Display some messages
  module_terminal_print_char('H', &t);
  module_terminal_print_c_string("ello, World!\n"
    "Welcome to the kernel created "
    "by Tancredi-Paul Grozav <paul@grozav.info>.\n"
    , &t);

  // Print integers
  uint8_t i1 = 215;
  module_terminal_print_uint8(i1, &t);
  module_serial_test();
}
// -------------------------------------------------------------------------- //

