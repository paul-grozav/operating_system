// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE_TERMINAL_H
#define MODULE_TERMINAL_H
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include <stdint.h> // uintX
// -------------------------------------------------------------------------- //
/**
 * Terminal structure
 */
typedef struct
{
  /**
   * This is the x86's VGA textmode buffer. To display text, we write data to
   * this memory location.
   */
  volatile uint16_t *buffer;

  /**
   * By default, the VGA textmode buffer has a size of 80x25 characters. You
   * should not change this value after creating the struct. This variable
   * contains the number of characters we can fit in one horizontal row/line of
   * text (80).
   */
  uint8_t total_columns;

  /**
   * By default, the VGA textmode buffer has a size of 80x25 characters. You
   * should not change this value after creating the struct. This variable
   * contains the number of rows/lines can fit on the screen (25).
   */
  uint8_t total_rows;

  /**
   * We start displaying text in the top-left of the screen (column=0, row=0).
   * This contains the current column where the cursor is at. Where the next
   * character will be placed.
   */
  uint8_t column_current;

  /**
   * We start displaying text in the top-left of the screen (column=0, row=0).
   * This contains the current row where the cursor is at. Where the next
   * character will be placed.
   */
  uint8_t row_current;

  //! Black background, White foreground
  uint8_t color;
} module_terminal_vga;

extern module_terminal_vga *module_terminal_vga_instance;

/**
 * Create a terminal_vga structure.
 */
module_terminal_vga module_terminal_vga_create();

//! This function initiates the global terminal by clearing it.
void module_terminal_global_init();

//! This function initiates the given terminal by clearing it.
void module_terminal_init(module_terminal_vga *t);

//! Print one character to the global screen.
void module_terminal_global_print_char(const char c);

//! Print one character to the given screen.
void module_terminal_print_char(const char c, module_terminal_vga *t);

//! This function prints an entire string onto the global screen.
void module_terminal_global_print_c_string(const char *str);

//! This function prints an entire string onto the given screen.
void module_terminal_print_c_string(const char *str, module_terminal_vga *t);

//! Print unsigned 8 bit integer on the global screen.
void module_terminal_global_print_uint8(const uint8_t i);

//! Print unsigned 8 bit integer on the given screen.
void module_terminal_print_uint8(const uint8_t i, module_terminal_vga *t);

//! Print unsigned 64 bit integer on the global screen.
void module_terminal_global_print_uint64(const uint64_t i);

//! Print unsigned 64 bit integer on the given screen.
void module_terminal_print_uint64(const uint64_t i, module_terminal_vga *t);

//! Print unsigned 64 bit integer as hex(base 16) on the global screen.
void module_terminal_global_print_hex_uint64(const uint64_t i);

//! Print unsigned 64 bit integer as hex(base 16) on the given screen.
void module_terminal_print_hex_uint64(const uint64_t i, module_terminal_vga *t);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
