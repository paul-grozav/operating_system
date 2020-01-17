// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include <stdint.h> // uintX
// -------------------------------------------------------------------------- //
//! This function initiates the terminal by clearing it
void terminal_init(volatile uint16_t* vga_buffer, const uint8_t VGA_COLS,
  const uint8_t VGA_ROWS, uint8_t term_color);

//! This function prints an entire string onto the screen
void terminal_print(const char* str, volatile uint16_t* vga_buffer,
  const uint8_t VGA_COLS, const uint8_t VGA_ROWS, uint8_t term_color,
  uint8_t *term_col, uint8_t *term_row);
// -------------------------------------------------------------------------- //

