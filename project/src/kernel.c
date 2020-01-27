// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// GCC provides these header files automatically
// They give us access to useful things like fixed-width types
#include <stdint.h>
#include "module_terminal.h"
#include "module_serial.h"
#include "module_base.h"
// -------------------------------------------------------------------------- //
// First, let's do some basic checks to make sure we are using our x86-elf
// cross-compiler correctly
#if defined(__linux__)
  #error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
  #error "This code must be compiled with an x86-elf compiler"
#endif
// -------------------------------------------------------------------------- //
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
  module_terminal_print_char('\n', &t);

  uint64_t i2 = 18446744073709551614;
  module_terminal_print_uint64(i2, &t);
  module_terminal_print_char('\n', &t);

  module_terminal_print_c_string(
    "64222 as hex is ", &t);
  module_terminal_print_hex_uint64(64222, &t); // 0xfade
  module_terminal_print_char('\n', &t);

  module_terminal_print_c_string(
    "kernel_main() at memory location: ", &t);
  module_terminal_print_hex_uint64(&kernel_main, &t);
  module_terminal_print_char('\n', &t);
  module_serial_test();
  // ------------- graphics test ---------------------------
  //mov $0x0e, %ah
  //mov $0x57, %al # 57=W
  //int $0x10
/*
  asm volatile(
    "mov $0x0e, %ah\n\t"
    "mov $0x57, %al\n\t"
    "int $0x10"
    "hlt"
  );
*/
// : "=a"(ret) : "Nd"(port));
  //-----------------
//  unsigned char *vram = 0xA0000;
//  module_terminal_print_hex_uint64(vram, &t);
//  module_terminal_print_char('\n', &t);
//  unsigned char *pixel = vram;// + y*pitch + x*pixelwidth;
//  *pixel = 4;
}
// -------------------------------------------------------------------------- //

