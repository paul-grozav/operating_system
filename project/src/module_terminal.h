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
} terminal_vga;

/**
 * Create a terminal_vga structure.
 */
terminal_vga terminal_vga_create();

//! This function initiates the terminal by clearing it
void terminal_init(terminal_vga *t);

//! This function prints an entire string onto the screen
void terminal_print(const char *str, terminal_vga *t);
// -------------------------------------------------------------------------- //

