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
// -------------------------------------------------------------------------- //
//! Test interrupts
void module_interrupts_test();
// -------------------------------------------------------------------------- //

