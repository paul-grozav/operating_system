// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
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

/**
 * Create a terminal_vga structure.
 */
module_terminal_vga module_terminal_vga_create();

//! This function initiates the terminal by clearing it
void module_terminal_init(module_terminal_vga *t);

//! Print one character to the screen
void module_terminal_print_char(const char c, module_terminal_vga *t);

//! This function prints an entire string onto the screen
void module_terminal_print_c_string(const char *str, module_terminal_vga *t);

//! Print unsigned 8 bit integer on the screen
void module_terminal_print_uint8(const uint8_t i, module_terminal_vga *t);

//! Print unsigned 64 bit integer on the screen
void module_terminal_print_uint64(const uint64_t i, module_terminal_vga *t);
// -------------------------------------------------------------------------- //

