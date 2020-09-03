// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_terminal.h"
#include "module_kernel.h" // in/out byte
#include "module_interrupt.h"
// -------------------------------------------------------------------------- //
// GDT - Global Descriptor Table
// -------------------------------------------------------------------------- //
struct module_interrupt_gdt_entry
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
}__attribute__((packed));
// -------------------------------------------------------------------------- //
struct module_interrupt_gdt_ptr
{
  uint16_t limit;
  uint32_t base;
}__attribute__((packed));
// -------------------------------------------------------------------------- //
//! This is defined in ASM - will register the table
extern void module_interrupt_gdt_flush(uint32_t);
// -------------------------------------------------------------------------- //
struct module_interrupt_gdt_entry module_interrupt_gdt_entries[5];
struct module_interrupt_gdt_ptr module_interrupt_gdt_ptr;
// -------------------------------------------------------------------------- //
void module_interrupt_gdt_set_gate(int32_t num, uint32_t base, uint32_t limit,
  uint8_t access, uint8_t granularity)
{
  module_interrupt_gdt_entries[num].base_low = (base & 0xFFFF);
  module_interrupt_gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  module_interrupt_gdt_entries[num].base_high = (base >> 24) & 0xFF;

  module_interrupt_gdt_entries[num].limit_low = (limit & 0xFFFF);
  module_interrupt_gdt_entries[num].granularity = (limit >> 16) & 0x0F;

  module_interrupt_gdt_entries[num].granularity |= granularity & 0xF0;
  module_interrupt_gdt_entries[num].access = access;
}
// -------------------------------------------------------------------------- //
void module_interrupt_init_gdt()
{
  module_interrupt_gdt_ptr.limit =
    (sizeof(struct module_interrupt_gdt_entry) * 5) - 1;
  module_interrupt_gdt_ptr.base = (uint32_t)&module_interrupt_gdt_entries;
  module_interrupt_gdt_set_gate(0,0,0,0,0);
  module_interrupt_gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
  module_interrupt_gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
  module_interrupt_gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
  module_interrupt_gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
  module_interrupt_gdt_flush((uint32_t)&module_interrupt_gdt_ptr);
}
// -------------------------------------------------------------------------- //
// IDT - Interrupt Descriptor Table
// -------------------------------------------------------------------------- //
struct module_interrupt_idt_entry
{
  uint16_t base_low;
  uint16_t selector;
  uint8_t always_zero;
  uint8_t flags;
  uint16_t base_high;
}__attribute__((packed));
// -------------------------------------------------------------------------- //
/**
 * Holds pointer interrupts table
 */
struct module_interrupt_idt_ptr
{
  //! How many interrupt entries there are
  uint16_t limit;
  //! Pointer to first interrupt entry
  uint32_t base;
}__attribute__((packed));
// -------------------------------------------------------------------------- //
struct module_interrupt_idt_ptr module_interrupt_idt_ptr;
struct module_interrupt_idt_entry module_interrupt_idt_entries[256];
extern void module_interrupt_idt_flush(uint32_t);
void module_interrupt_init_idt();
// -------------------------------------------------------------------------- //
extern void module_interrupt_isr0();
extern void module_interrupt_isr1();
extern void module_interrupt_isr2();
extern void module_interrupt_isr3();
extern void module_interrupt_isr4();
extern void module_interrupt_isr5();
extern void module_interrupt_isr6();
extern void module_interrupt_isr7();
extern void module_interrupt_isr8();
extern void module_interrupt_isr9();
extern void module_interrupt_isr10();
extern void module_interrupt_isr11();
extern void module_interrupt_isr12();
extern void module_interrupt_isr13();
extern void module_interrupt_isr14();
extern void module_interrupt_isr15();
extern void module_interrupt_isr16();
extern void module_interrupt_isr17();
extern void module_interrupt_isr18();
extern void module_interrupt_isr19();
extern void module_interrupt_isr20();
extern void module_interrupt_isr21();
extern void module_interrupt_isr22();
extern void module_interrupt_isr23();
extern void module_interrupt_isr24();
extern void module_interrupt_isr25();
extern void module_interrupt_isr26();
extern void module_interrupt_isr27();
extern void module_interrupt_isr28();
extern void module_interrupt_isr29();
extern void module_interrupt_isr30();
extern void module_interrupt_isr31();
extern void module_interrupt_irq0 ();
extern void module_interrupt_irq1 ();
extern void module_interrupt_irq2 ();
extern void module_interrupt_irq3 ();
extern void module_interrupt_irq4 ();
extern void module_interrupt_irq5 ();
extern void module_interrupt_irq6 ();
extern void module_interrupt_irq7 ();
extern void module_interrupt_irq8 ();
extern void module_interrupt_irq9 ();
extern void module_interrupt_irq10();
extern void module_interrupt_irq11();
extern void module_interrupt_irq12();
extern void module_interrupt_irq13();
extern void module_interrupt_irq14();
extern void module_interrupt_irq15();
// -------------------------------------------------------------------------- //
/**
 * Set an interrupt handler for the given interrupt number.
 * This calls the ASM function, which will callback a C function.
 * @param[in] num - Interrupt number for which we set the handler
 * @param[in] base - The memory address where the handler resides. The execution
 * will jump here when the interrupt fires.
 * @param[in] selector - Always seems constant: 0x08
 * @param[in] flags - Always seems constant: 0x8E
 */
void module_interrupt_idt_set_gate(const uint8_t num, const uint32_t base,
  const uint16_t selector, const uint8_t flags)
{
  module_interrupt_idt_entries[num].base_low = base & 0xFFFF;
  module_interrupt_idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  module_interrupt_idt_entries[num].selector = selector;
  module_interrupt_idt_entries[num].always_zero = 0;
  module_interrupt_idt_entries[num].flags = flags | 0x60;
}
// -------------------------------------------------------------------------- //
void module_interrupt_init_idt()
{
  module_interrupt_idt_ptr.limit =
    sizeof(struct module_interrupt_idt_entry) * 256 - 1;
  module_interrupt_idt_ptr.base = (uint32_t)&module_interrupt_idt_entries;
  module_kernel_memset(&module_interrupt_idt_entries, 0,
    sizeof(struct module_interrupt_idt_entry) * 256);

  module_kernel_out_8(0x20, 0x11);
  module_kernel_out_8(0xA0, 0x11);
  module_kernel_out_8(0x21, 0x20);
  module_kernel_out_8(0xA1, 0x28);
  module_kernel_out_8(0x21, 0x04);
  module_kernel_out_8(0xA1, 0x02);
  module_kernel_out_8(0x21, 0x01);
  module_kernel_out_8(0xA1, 0x01);
  module_kernel_out_8(0x21, 0x0);
  module_kernel_out_8(0xA1, 0x0);

  // instructions for which functions should be called for which interrupts
  // Common practice is to leave the first 32 vectors for exceptions, as
  // mandated by Intel.
  module_interrupt_idt_set_gate(0,(uint32_t)module_interrupt_isr0,0x08,0x8E);
  module_interrupt_idt_set_gate(1,(uint32_t)module_interrupt_isr1,0x08,0x8E);
  module_interrupt_idt_set_gate(2,(uint32_t)module_interrupt_isr2,0x08,0x8E);
  module_interrupt_idt_set_gate(3,(uint32_t)module_interrupt_isr3,0x08,0x8E);
  module_interrupt_idt_set_gate(4,(uint32_t)module_interrupt_isr4,0x08,0x8E);
  module_interrupt_idt_set_gate(5,(uint32_t)module_interrupt_isr5,0x08,0x8E);
  module_interrupt_idt_set_gate(6,(uint32_t)module_interrupt_isr6,0x08,0x8E);
  module_interrupt_idt_set_gate(7,(uint32_t)module_interrupt_isr7,0x08,0x8E);
  module_interrupt_idt_set_gate(8,(uint32_t)module_interrupt_isr8,0x08,0x8E);
  module_interrupt_idt_set_gate(9,(uint32_t)module_interrupt_isr9,0x08,0x8E);
  module_interrupt_idt_set_gate(10,(uint32_t)module_interrupt_isr10,0x08,0x8E);
  module_interrupt_idt_set_gate(11,(uint32_t)module_interrupt_isr11,0x08,0x8E);
  module_interrupt_idt_set_gate(12,(uint32_t)module_interrupt_isr12,0x08,0x8E);
  module_interrupt_idt_set_gate(13,(uint32_t)module_interrupt_isr13,0x08,0x8E);
  module_interrupt_idt_set_gate(14,(uint32_t)module_interrupt_isr14,0x08,0x8E);
  module_interrupt_idt_set_gate(15,(uint32_t)module_interrupt_isr15,0x08,0x8E);
  module_interrupt_idt_set_gate(16,(uint32_t)module_interrupt_isr16,0x08,0x8E);
  module_interrupt_idt_set_gate(17,(uint32_t)module_interrupt_isr17,0x08,0x8E);
  module_interrupt_idt_set_gate(18,(uint32_t)module_interrupt_isr18,0x08,0x8E);
  module_interrupt_idt_set_gate(19,(uint32_t)module_interrupt_isr19,0x08,0x8E);
  module_interrupt_idt_set_gate(20,(uint32_t)module_interrupt_isr20,0x08,0x8E);
  module_interrupt_idt_set_gate(21,(uint32_t)module_interrupt_isr21,0x08,0x8E);
  module_interrupt_idt_set_gate(22,(uint32_t)module_interrupt_isr22,0x08,0x8E);
  module_interrupt_idt_set_gate(23,(uint32_t)module_interrupt_isr23,0x08,0x8E);
  module_interrupt_idt_set_gate(24,(uint32_t)module_interrupt_isr24,0x08,0x8E);
  module_interrupt_idt_set_gate(25,(uint32_t)module_interrupt_isr25,0x08,0x8E);
  module_interrupt_idt_set_gate(26,(uint32_t)module_interrupt_isr26,0x08,0x8E);
  module_interrupt_idt_set_gate(27,(uint32_t)module_interrupt_isr27,0x08,0x8E);
  module_interrupt_idt_set_gate(28,(uint32_t)module_interrupt_isr28,0x08,0x8E);
  module_interrupt_idt_set_gate(29,(uint32_t)module_interrupt_isr29,0x08,0x8E);
  module_interrupt_idt_set_gate(30,(uint32_t)module_interrupt_isr30,0x08,0x8E);
  module_interrupt_idt_set_gate(31,(uint32_t)module_interrupt_isr31,0x08,0x8E);

  // Interrupts table
  // Items are numbered from 0-15, but actual interrupt number is +32
  //-----------
  // IRQ  Description
  // 0    Programmable Interrupt Timer Interrupt
  // 1    Keyboard Interrupt
  // 2    Cascade (used internally by the two PICs. never raised)
  // 3    COM2 (if enabled)
  // 4    COM1 (if enabled)
  // 5    LPT2 (if enabled)
  // 6    Floppy Disk
  // 7    LPT1 / Unreliable "spurious" interrupt (usually)
  // 8    CMOS real-time clock (if enabled)
  // 9    Free for peripherals / legacy SCSI / NIC
  // 10   Free for peripherals / SCSI / NIC
  // 11   Free for peripherals / SCSI / NIC
  // 12   PS2 Mouse
  // 13   FPU / Coprocessor / Inter-processor
  // 14   Primary ATA Hard Disk
  // 15   Secondary ATA Hard Disk
  //-----------
  module_interrupt_idt_set_gate(32,(uint32_t)module_interrupt_irq0,0x08,0x8E);
  module_interrupt_idt_set_gate(33,(uint32_t)module_interrupt_irq1,0x08,0x8E);
  module_interrupt_idt_set_gate(34,(uint32_t)module_interrupt_irq2,0x08,0x8E);
  module_interrupt_idt_set_gate(35,(uint32_t)module_interrupt_irq3,0x08,0x8E);
  module_interrupt_idt_set_gate(36,(uint32_t)module_interrupt_irq4,0x08,0x8E);
  module_interrupt_idt_set_gate(37,(uint32_t)module_interrupt_irq5,0x08,0x8E);
  module_interrupt_idt_set_gate(38,(uint32_t)module_interrupt_irq6,0x08,0x8E);
  module_interrupt_idt_set_gate(39,(uint32_t)module_interrupt_irq7,0x08,0x8E);
  module_interrupt_idt_set_gate(40,(uint32_t)module_interrupt_irq8,0x08,0x8E);
  module_interrupt_idt_set_gate(41,(uint32_t)module_interrupt_irq9,0x08,0x8E);
  module_interrupt_idt_set_gate(42,(uint32_t)module_interrupt_irq10,0x08,0x8E);
  module_interrupt_idt_set_gate(43,(uint32_t)module_interrupt_irq11,0x08,0x8E);
  module_interrupt_idt_set_gate(44,(uint32_t)module_interrupt_irq12,0x08,0x8E);
  module_interrupt_idt_set_gate(45,(uint32_t)module_interrupt_irq13,0x08,0x8E);
  module_interrupt_idt_set_gate(46,(uint32_t)module_interrupt_irq14,0x08,0x8E);
  module_interrupt_idt_set_gate(47,(uint32_t)module_interrupt_irq15,0x08,0x8E);

  // Set interrupts
  module_interrupt_idt_flush((uint32_t)&module_interrupt_idt_ptr);
}
// -------------------------------------------------------------------------- //
// INTERRUPTS (IRQ)
// -------------------------------------------------------------------------- //
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47
// -------------------------------------------------------------------------- //
module_interrupt_registers_t module_interrupt_registers;
// -------------------------------------------------------------------------- //
module_interrupt_isr_t module_interrupt_interrupt_handlers[256];
// -------------------------------------------------------------------------- //
void module_interrupt_register_interrupt_handler(const uint8_t n,
  const module_interrupt_isr_t handler)
{
  module_interrupt_interrupt_handlers[n] = handler;
}
// -------------------------------------------------------------------------- //
void module_interrupt_isr_handler(module_interrupt_registers_t regs)
{
  module_terminal_global_print_c_string("isr_handler: Received interrupt:");
  module_terminal_global_print_uint64(regs.int_no);
  module_terminal_global_print_char('\n');
}
// -------------------------------------------------------------------------- //
void module_interrupt_irq_handler(module_interrupt_registers_t regs)
{
//  module_terminal_global_print_c_string("irq_handler: Received interrupt:");
//  module_terminal_global_print_uint64(regs.int_no);
//  module_terminal_global_print_char('\n');


  if (module_interrupt_interrupt_handlers[regs.int_no] != 0 )
  {
    module_interrupt_isr_t handler =
      module_interrupt_interrupt_handlers[regs.int_no];
    handler(regs);
  }
  else
  {
    module_terminal_global_print_c_string("irq_handler: No handler registered"
      " for interrupt: ");
    module_terminal_global_print_uint64(regs.int_no);
    module_terminal_global_print_c_string(" !\n");
  }

  // This is part of EOI.
  // we should do this AFTER calling the handler - not before
  if (regs.int_no >= 40)
  {
    module_kernel_out_8(0xA0,0x20);
  }
  module_kernel_out_8(0x20,0x20);//EOI = End-Of-Interrupt
  // Enable  interrupts so that other could be handled
  //module_interrupt_enable();
}
// -------------------------------------------------------------------------- //
void module_interrupt_init()
{
  // init GDT before IDT. IDT requires GDT
  module_interrupt_init_gdt();
  module_interrupt_init_idt();
}
// -------------------------------------------------------------------------- //
void module_interrupt_test()
{
  // make sure you've called module_interrupt_init() first.
  module_terminal_global_print_char('\n');
  asm volatile ("int $0x3"); // call software interrupt 3
}
// -------------------------------------------------------------------------- //
void module_interrupt_enable()
{
  asm volatile ("sti");
}
// -------------------------------------------------------------------------- //
void module_interrupt_disable()
{
  asm volatile ("cli");
}
// -------------------------------------------------------------------------- //
void module_interrupt_enable_irq(const uint8_t irq)
{
  const uint8_t pic_master_data = 33; // 0x21
  const uint8_t pic_slave_data = 161; // 0xA1

  static uint16_t ocw1 = 0xFFFB;
  ocw1 &= (uint16_t)~((1 << (irq-32)));

  if ((irq-32) < 8)
  {
    module_kernel_out_8(pic_master_data, (uint8_t)(ocw1 & 0xFF));
  }
  else
  {
    module_kernel_out_8(pic_slave_data, (uint8_t)(ocw1 >> 8));
  }
}
// -------------------------------------------------------------------------- //

