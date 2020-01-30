// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// GCC provides these header files automatically
// They give us access to useful things like fixed-width types
#include <stdint.h>
#include "module_terminal.h"
#include "module_serial.h"
#include "module_base.h"
#include "module_interrupt.h"
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

  // Create the global terminal instance
  module_terminal_vga terminal = module_terminal_vga_create();
  module_terminal_vga_instance = &terminal;
  // Initiate(clear) terminal
  module_terminal_global_init();
//  asm volatile ("hlt"); // halt cpu

  // Display some messages
  module_terminal_global_print_char('H');
  module_terminal_global_print_c_string("ello, World!\n"
    "Welcome to the kernel created by "
    "Tancredi-Paul Grozav <paul@grozav.info>.\n"
  );

  // Print integers
  module_terminal_global_print_char('\n');
  uint8_t i1 = 215;
  module_terminal_global_print_uint8(i1);
  module_terminal_global_print_char('\n');

  uint64_t i2 = 18446744073709551614;
  module_terminal_global_print_uint64(i2);
  module_terminal_global_print_char('\n');

  module_terminal_global_print_c_string("64222 as hex is ");
  module_terminal_global_print_hex_uint64(64222); // 0xfade
  module_terminal_global_print_char('\n');

  module_terminal_global_print_c_string("kernel_main() at memory location: ");
  module_terminal_global_print_hex_uint64(
    (uint64_t)(uint32_t)(&kernel_main));
  module_terminal_global_print_char('\n');

  module_terminal_global_print_c_string("Running serial_test ...");
  module_serial_test();
  module_terminal_global_print_c_string(" Done.\n");
  //-----------------
  // IDT
  // http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
/*
  module_terminal_print_char('\n');
  uint16_t total;
  uint8_t low_mem=0;
  uint8_t high_mem=0;

  outb(0x70, 0x30);
  low_mem = inb(0x71);
  outb(0x70, 0x31);
  high_mem = inb(0x71);

  total = low_mem | high_mem << 8;
  module_terminal_print_c_string("low_mem=");
  module_terminal_print_uint64(low_mem);
  module_terminal_print_char('\n');
  module_terminal_print_c_string("high_mem=");
  module_terminal_print_uint64(high_mem);
  module_terminal_print_char('\n');
  module_terminal_print_c_string("total_mem=");
  module_terminal_print_uint64(total);
  module_terminal_print_char('\n');
*/
  module_terminal_global_print_c_string("Running interrupts_test ...");
  module_interrupts_test();
  module_terminal_global_print_c_string(" Done.\n");

  module_terminal_global_print_c_string("\n-------------\n");
  module_terminal_global_print_c_string("Kernel ended. B`bye!");
}
// -------------------------------------------------------------------------- //

