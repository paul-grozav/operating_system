// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// GCC provides these header files automatically
// They give us access to useful things like fixed-width types
#include <stdint.h>
#include <stdbool.h>
#include "module_terminal.h"
#include "module_serial.h"
#include "module_base.h"
#include "module_interrupt.h"
#include "module_heap.h"
#include "module_video.h"
#include "module_keyboard.h"
#include "module__pci.h"
#include "module__network.h"
#include "module__network__ethernet_interface.h"
#include "module__driver__rtl8139.h"
//#include "test.hpp"
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
// should be declared as extern "C" if written in C++ to be callable from ASM
void kernel_main()
{
  // We're here! Let's initiate the terminal and display a message to show we
  // got here.

  // Create the global terminal instance
  module_terminal_vga terminal = module_terminal_vga_create();
  module_terminal_vga_instance = &terminal;
  // Initiate(clear) terminal
  module_terminal_global_init(1);
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

  // U suffix avoids warning: integer constant is so large that it is unsigned
  uint64_t i2 = 18446744073709551614U;
  module_terminal_global_print_uint64(i2);
  module_terminal_global_print_char('\n');

  module_terminal_global_print_c_string("64222 as hex is ");
  module_terminal_global_print_hex_uint64(64222); // 0xfade
  module_terminal_global_print_char('\n');

  module_terminal_global_print_c_string("65535 as binary is ");
  module_terminal_global_print_binary_uint64(65535); // 0xfade
  module_terminal_global_print_char('\n');

  module_terminal_global_print_c_string("kernel_main() at memory location: ");
  module_terminal_global_print_hex_uint64(
    (uint64_t)(uint32_t)(&kernel_main));
  module_terminal_global_print_char('\n');
// -------------------------------------------------------------------------- //
  module_terminal_global_print_c_string("Running serial_test ...");
  module_serial_test();
  module_terminal_global_print_c_string(" Done.\n");
// -------------------------------------------------------------------------- //
  module_terminal_global_print_c_string("Running interrupts_test ...");
  module_interrupt_disable();
  module_interrupt_init();
  module_interrupt_test();
  module_terminal_global_print_c_string(" Done.\n");
// -------------------------------------------------------------------------- //
  module_terminal_global_print_c_string("Running heap tests and init ...");
  module_heap_test();
  module_heap_heap_instance_init(); // init main kernel heap instance
  module_terminal_global_print_c_string(" Done.\n");
// -------------------------------------------------------------------------- //
  module_terminal_global_print_c_string("Enabling keyboard...\n");
  module_keyboard_enable();
  module_terminal_global_print_c_string("Enabling interrupts...\n");
  module_interrupt_enable();
// -------------------------------------------------------------------------- //
//  module_video_test();
  module__pci__detect_devices();
  module__pci__test();
  module__network__ethernet_interface__init_all();
//  module__driver__rtl8139__test();
//  module_terminal_global_print_c_string("Press any key to send NIC data...");
//  module_keyboard_wait_keypress();
  module__network__test();
//  test_cpp();

// -------------------------------------------------------------------------- //
  // avoid cpu hanging. wait for keyboard input -- uses 1 CPU core to 100%
//  while(true)
//  {
//    module__network__queue__process();
//  }

  module_terminal_global_print_c_string("Press any key to end the kernel...");
  module_keyboard_wait_keypress();
  module_terminal_global_print_c_string("\n");

  module_terminal_global_print_c_string("Freeing Ethernet interfaces list"
    " ...\n");
  module__network__ethernet_interface__free_all();
  module_terminal_global_print_c_string("Done freeing Ethernet interfaces"
    " list.\n");

  module_terminal_global_print_c_string("Freeing RTL8139 drivers list ...\n");
  module__driver__rtl8139__driver_free_all();
  module_terminal_global_print_c_string("Done freeing RTL8139 drivers list.\n");

  module_terminal_global_print_c_string("Freeing PCI devices list ...\n");
  module__pci__free_devices();
  module_terminal_global_print_c_string("Done freeing PCI devices list.\n");

  module_terminal_global_print_c_string("\n-------------\n");
  module_terminal_global_print_c_string("Kernel ended. B`bye!\n");
  module_terminal_global_print_c_string("You can stop your machine now.");
}
// -------------------------------------------------------------------------- //
// Junk follows
// -------------------------------------------------------------------------- //
/*
#define IRQ_BASE 0x20
#define PIC_MASTER_CTRL 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_DATA  0xA1
#define PIC_SLAVE_CTRL 0xA0

//  pic();
void
pic(void) {

    // ICW1
    outportb(PIC_MASTER_CTRL, 0x11);  // init master PIC
    outportb(PIC_SLAVE_CTRL, 0x11);   // init slave PIC
    // ICW2
    outportb(PIC_MASTER_DATA, 0x20);  // IRQ 0..7 remaped to 0x20..0x27
    outportb(PIC_SLAVE_DATA, 0x28);   // IRQ 8..15 remaped to 0x28..0x37
    // ICW3
    outportb(PIC_MASTER_DATA, 0x04);  // set as Master
    outportb(PIC_SLAVE_DATA, 0x02);   // set as Slave
    // ICW4
    outportb(PIC_MASTER_DATA, 0x01);  // set x86 mode
    outportb(PIC_SLAVE_DATA, 0x01);   // set x86 mode

    outportb(PIC_MASTER_DATA, 0xFF);  // all interrupts disabled
    outportb(PIC_SLAVE_DATA, 0xFF);

    __asm__ __volatile__("nop");
}
*/
// -------------------------------------------------------------------------- //

