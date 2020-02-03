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
#include "module_heap.h"
#include "module_video.h"
// -------------------------------------------------------------------------- //
// First, let's do some basic checks to make sure we are using our x86-elf
// cross-compiler correctly
#if defined(__linux__)
  #error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
  #error "This code must be compiled with an x86-elf compiler"
#endif
// -------------------------------------------------------------------------- //
#define outportb(a,b) module_kernel_out_8(a,b)
void waitch()
{
  int key;
  while ( 1 )
  {
/*
    module_kernel_out_8(0x20, 0x20); // Send EOI
    unsigned char c = module_kernel_in_8( 0x60 );
    if((c & 128) == 128)
      module_terminal_global_print_c_string("RELEASE\n");
    else
      module_terminal_global_print_c_string("PRESS\n");
*/
///
    // wait for key
    while ((module_kernel_in_8(0x64) & 1) == 0);
    key = module_kernel_in_8( 0x60 ); // same as inb- use yours
    module_terminal_global_print_uint64(key);
    if ( key & 0x80 ) continue;
    if ( key != 0 ) return;
    else if ( key == 0 ) continue;
//*/
  }
}

#define IRQ_BASE 0x20
#define PIC_MASTER_CTRL 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CTRL 0xA0
#define PIC_SLAVE_DATA  0xA1

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


static uint16_t ocw1 = 0xFFFB;

void irq_enable(uint8_t irq)
{
	ocw1 &= (uint16_t)~((1 << irq));

	if (irq < 8)
		outportb(PIC_MASTER_DATA, (uint8_t)(ocw1 & 0xFF));
	else
		outportb(PIC_SLAVE_DATA, (uint8_t)(ocw1 >> 8));
}

void irq1_handler(module_interrupt_registers_t x)
{
  module_terminal_global_print_c_string("IRQ1 handler\n");
}

























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

  module_terminal_global_print_c_string("Running interrupts_test ...");
  module_interrupt_disable();
  module_interrupt_init();
  module_interrupt_test();
  module_terminal_global_print_c_string(" Done.\n");

  // heap begin
  module_heap_heap_bm kheap;
  char *ptr;

  // initialize the heap
  module_terminal_global_print_c_string("Initialize heap...\n");
  module_heap_init(&kheap);

  // add block to heap
  // starting 10MB mark and length of 1MB with default block size of 16 bytes
  module_terminal_global_print_c_string("Add heap block...\n");
  module_heap_add_block(&kheap, 10*1024*1024, 1024*1024, 16);

  // allocate 256 bytes (malloc)
  module_terminal_global_print_c_string("Heap alloc...\n");
  ptr = (char*)module_heap_alloc(&kheap, 256);

  // free the pointer (free)
  module_terminal_global_print_c_string("Heap free...\n");
  module_heap_free(&kheap, ptr);
  // heap end

  //-----------------
  pic();
//  IRQ_SET_HANDLER(1, keyboard_handler);
  module_interrupt_register_interrupt_handler(1, irq1_handler);
  irq_enable(1);
//  waitch();
//  module_video_test(&kheap);
  module_interrupt_enable();
  while(1);//avoid hang.wait kb

  module_terminal_global_print_c_string("\n-------------\n");
  module_terminal_global_print_c_string("Kernel ended. B`bye!");
}
// -------------------------------------------------------------------------- //

