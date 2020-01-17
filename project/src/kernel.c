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
  // This is the x86's VGA textmode buffer. To display text, we write data to
  // this memory location
  volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
  // By default, the VGA textmode buffer has a size of 80x25 characters
  const uint8_t VGA_COLS = 80;
  const uint8_t VGA_ROWS = 25;

  // We start displaying text in the top-left of the screen (column=0, row=0)
  uint8_t term_col = 0;
  uint8_t term_row = 0;
  uint8_t term_color = 0x0F; // Black background, White foreground

  // We're here! Let's initiate the terminal and display a message to show we
  // got here.

  // Initiate terminal
  terminal_init(vga_buffer, VGA_COLS, VGA_ROWS, term_color);

  // Display some messages
  terminal_print("Hello, World!\n", vga_buffer, VGA_COLS, VGA_ROWS, term_color,
    &term_col, &term_row);
  terminal_print("Welcome to the kernel created "
    "by Tancredi-Paul Grozav <paul@grozav.info>.\n",
    vga_buffer, VGA_COLS, VGA_ROWS, term_color, &term_col, &term_row);
}
// -------------------------------------------------------------------------- //

