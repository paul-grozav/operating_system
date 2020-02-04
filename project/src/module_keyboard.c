// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include "module_keyboard.h"
#include "module_kernel.h"
#include "module_terminal.h"
// -------------------------------------------------------------------------- //
volatile uint8_t ScanCode;
void module_keyboard_interrupt_handler(module_interrupt_registers_t x)
{
//  module_terminal_global_print_c_string("IRQ1 handler\n");
  module_kernel_out_8(0x20, 0x20);// Send End-of-interrupt
  ScanCode = module_kernel_in_8(0x60);
  // MSB 1000000 tells us if it was a key up or down
  // the other bits describe the key
  if((ScanCode & 128) == 128)
  {
    return;
    module_terminal_global_print_c_string("Released");
  }
  else
  {
    module_terminal_global_print_c_string("Pressed");
  }
  module_terminal_global_print_char(' ');
  module_terminal_global_print_uint64(ScanCode);
//  module_terminal_global_print_char('=');
//  module_terminal_global_print_uint64(ScanCode & ~(0<<7));
  module_terminal_global_print_char(' ');
  uint8_t c = 0;
  if(ScanCode == 30) //0x1E)
  {
    c = 'A';
  }
  else if(ScanCode == 48) //0x30)
  {
    c = 'B';
  }
  else if(ScanCode == 46) //0x21)
  {
    c = 'C';
  }
  module_terminal_global_print_c_string("character '");
  module_terminal_global_print_char(c);
  module_terminal_global_print_c_string("'\n");
  module_interrupt_enable();
}
// -------------------------------------------------------------------------- //
void module_keyboard_enable()
{
  module_interrupt_register_interrupt_handler(32+1,
    module_keyboard_interrupt_handler);
  module_interrupt_enable_irq(32+1);
}
// -------------------------------------------------------------------------- //

