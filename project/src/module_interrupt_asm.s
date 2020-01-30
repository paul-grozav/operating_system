// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// Interrupts (IRQ) requires GDT + IDT + ISR
// -------------------------------------------------------------------------- //
// GDT
.section .text
.align 4

.global gdt_flush
.type gdt_flush, @function

gdt_flush:
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
// IDT
.section .text
.align 4

.global idt_flush
.type idt_flush, @function

idt_flush:
  mov 4(%esp),%eax
  lidt (%eax)
  ret
// -------------------------------------------------------------------------- //
// Interrupt Service Routine - a.k.a. interrupt handler
.section .text
.align 4

.macro ISR_NOERR index
  .global isr\index
  isr\index:
    cli
    push $0
    push $\index
    jmp isr_common
.endm

.macro ISR_ERR index
  .global isr\index
  isr\index:
    cli
    push $\index
    jmp isr_common
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

.extern isr_handler
.type isr_handler, @function

isr_common:
  pusha
  mov %ds,%ax
  push %eax
  mov $0x10, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %fs
  mov %ax, %gs

  call isr_handler

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
  .global irq\ident
  .type irq\ident, @function
  irq\ident:
    cli
    push $0x00
    push $\byte
    jmp irq_common
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

.global irq_handler
.type irq_handler, @function

irq_common:
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
  call irq_handler
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
// -------------------------------------------------------------------------- //

