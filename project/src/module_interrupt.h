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
//! Structure received by interrupt handler
typedef struct
{
  //! Data segment selector
  uint32_t ds;

  //! Pushed by pusha.
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

  //! Interrupt number and error code (if applicable)
  uint32_t int_no, err_code;

  //! Pushed by the processor automatically.
  uint32_t eip, cs, eflags, useresp, ss;
} module_interrupt_registers_t;

//! Initiate interrupt handlers.
void module_interrupt_init();

//! Test interrupts
void module_interrupt_test();

//! Enable interrupts. Allow the CPU to be interrupted.
void module_interrupt_enable();

//! Disable interrupts. Do not allow the CPU to be interrupted.
void module_interrupt_disable();
// -------------------------------------------------------------------------- //

