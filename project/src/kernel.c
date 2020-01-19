// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// GCC provides these header files automatically
// They give us access to useful things like fixed-width types
#include <stdint.h>
#include "module_terminal.h"
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

  terminal_vga t = terminal_vga_create();
  // Initiate terminal
  terminal_init(&t);

  // Display some messages
  terminal_print("Hello, World!\n", &t);
  terminal_print("Welcome to the kernel created "
    "by Tancredi-Paul Grozav <paul@grozav.info>.\n", &t);
}
// -------------------------------------------------------------------------- //

