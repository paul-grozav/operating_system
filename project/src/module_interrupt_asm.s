// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// Interrupts (IRQ) requires GDT + IDT + ISR
// -------------------------------------------------------------------------- //
// GDT - Global Descriptor Table
.section .text
.align 4

.global module_interrupt_gdt_flush
.type module_interrupt_gdt_flush, @function

module_interrupt_gdt_flush:
  mov 4(%esp), %eax
  lgdt (%eax)

  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %ss
  mov %ax, %gs

  ljmp $0x8, $.flush
.flush:
  ret
// -------------------------------------------------------------------------- //
// IDT - Interrupt Descriptor Table - contains handlers for all interrupts
.section .text
.align 4

//! This function sets the given interrupt handlers table, so that the machine
//! knows which handler to call for each interrupt
.global module_interrupt_idt_flush
.type module_interrupt_idt_flush, @function

module_interrupt_idt_flush:
  mov 4(%esp),%eax
  lidt (%eax)
  ret
// -------------------------------------------------------------------------- //
// Interrupt Service Routine - a.k.a. interrupt handler - defines handlers
// for each interrupt
.section .text
.align 4

.macro ISR_NOERR index
  .global module_interrupt_isr\index
  module_interrupt_isr\index:
    cli
    push $0
    push $\index
    jmp module_interrupt_isr_common
.endm

.macro ISR_ERR index
  .global module_interrupt_isr\index
  module_interrupt_isr\index:
    cli
    push $\index
    jmp module_interrupt_isr_common
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31
//ISR_NOERR 127

.extern module_interrupt_isr_handler
.type module_interrupt_isr_handler, @function

module_interrupt_isr_common:
  pusha
  mov %ds,%ax
  push %eax
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  call module_interrupt_isr_handler

  pop %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  popa
  add $8, %esp
  iret
// -------------------------------------------------------------------------- //
// IRQs
.section .text
.align 4

.macro IRQ ident byte
  .global module_interrupt_irq\ident
  .type module_interrupt_irq\ident, @function
  module_interrupt_irq\ident:
    cli
    push $0x00
    push $\byte
    jmp module_interrupt_irq_common
.endm

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

// Making it global, allows the C code to call this function
.global module_interrupt_irq_handler
.type module_interrupt_irq_handler, @function

// bad ?
/*
module_interrupt_irq_common:
  pusha

  push %ds
  push %es
  push %fs
  push %gs
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs
  cld

  // Call interrupt handler
  push %esp
  call module_interrupt_irq_handler
  add $4, %esp

  // Restore segment registers
  pop %gs
  pop %fs
  pop %es
  pop %ds

  // Restore all registers
  popa
  // Cleanup error code and IRQ #
  add $8, %esp
  // pop CS, EIP, EFLAGS, SS and ESP
  iret
*/


// This is our common IRQ stub. It saves the processor state, sets
// up for kernel mode segments, calls the C-level fault handler,
// and finally restores the stack frame.
module_interrupt_irq_common:
  // Pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
  pusha

  // Lower 16-bits of eax = ds.
  mov %ds,%ax

  // save the data segment descriptor
  push %eax

  // load the kernel data segment descriptor
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  // Call C handler
  call module_interrupt_irq_handler

  // reload the original data segment descriptor
  pop %eax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  // Pop edi, esi, ebp, esp, ebx, edx, ecx, eax
  popa

  // Cleans up the pushed error code and pushed ISR number
  add $8, %esp
  //sti // enable interrupts again ?
  // pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
  iret
// -------------------------------------------------------------------------- //

