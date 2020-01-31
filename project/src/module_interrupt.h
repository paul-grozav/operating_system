// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// This module is composed of this header, it's corresponding .c file, and an
// extra _asm.s file.
// An operating system has to register interrupt handlers because the CPU will
// sometimes need to signal the kernel(using these interrupts)(for example,
// division by zero), and if handlers are not registered, the CPU will triple
// fault and reset.
//
// This module defines GDT & IDT + ISR
//
// When interrupt 3(for example) is fired, the first function called is:
// 1. ASM: module_interrupt_isr3 (defined by ISR_NOERR 3), which calls:
// 2. ASM: module_interrupt_isr_common, which calls:
// 3. C: module_interrupt_isr_handler which handles the interrupt
//
// That first ASM call: module_interrupt_isr3 is fired because init_idt sets
// the handlers for all interrupts. All the handlers are defined in this table:
// module_interrupt_idt_entries and are registered by this ASM call:
// module_interrupt_idt_flush .
// -------------------------------------------------------------------------- //
//! Initiate interrupt handlers.
void module_interrupt_init();

//! Test interrupts
void module_interrupt_test();
// -------------------------------------------------------------------------- //

