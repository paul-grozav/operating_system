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
   * should not change the values of these members
   */
  uint8_t columns;
  uint8_t rows;

  //! We start displaying text in the top-left of the screen (column=0, row=0)
  uint8_t term_col;
  uint8_t term_row;
  
  //! Black background, White foreground
  uint8_t term_color;
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

