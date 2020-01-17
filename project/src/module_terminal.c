// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_terminal.h"
// -------------------------------------------------------------------------- //
void terminal_init(volatile uint16_t* vga_buffer, const uint8_t VGA_COLS,
  const uint8_t VGA_ROWS, uint8_t term_color)
{
  // Clear the textmode buffer
  for (int col = 0; col < VGA_COLS; col ++)
  {
    for (int row = 0; row < VGA_ROWS; row ++)
    {
      // The VGA textmode buffer has size (VGA_COLS * VGA_ROWS).
      // Given this, we find an index into the buffer for our character
      const size_t index = (VGA_COLS * row) + col;
      // Entries in the VGA buffer take the binary form BBBBFFFFCCCCCCCC, where:
      // - B is the background color
      // - F is the foreground color
      // - C is the ASCII character
      // Set the character to blank (a space character)
      vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
    }
  }
}
// -------------------------------------------------------------------------- //
// This function places a single character onto the screen
void terminal_putc(char c, volatile uint16_t* vga_buffer,
  const uint8_t VGA_COLS, const uint8_t VGA_ROWS, uint8_t term_color,
  uint8_t *term_col, uint8_t *term_row)
{
  // Remember - we don't want to display ALL characters!
  switch (c)
  {
    case '\n':
    { // Newline characters should return the column to 0, and increment the row
      (*term_col) = 0;
      (*term_row) ++;
      break;
    }

    default:
    { // Normal characters just get displayed and then increment the column
      // Like before, calculate the buffer index
      const size_t index = (VGA_COLS * (*term_row)) + (*term_col);
      vga_buffer[index] = ((uint16_t)term_color << 8) | c;
      (*term_col) ++;
      break;
    }
  }
  // What happens if we get past the last column? We need to reset the column to
  // 0, and increment the row to get to a new line
  if ((*term_col) >= VGA_COLS)
  {
    (*term_col) = 0;
    (*term_row) ++;
  }
  // What happens if we get past the last row? We need to reset both column and
  // row to 0 in order to loop back to the top of the screen
  if ((*term_row) >= VGA_ROWS)
  {
    (*term_col) = 0;
    (*term_row) = 0;
  }
}
// -------------------------------------------------------------------------- //
void terminal_print(const char* str, volatile uint16_t* vga_buffer,
  const uint8_t VGA_COLS, const uint8_t VGA_ROWS, uint8_t term_color,
  uint8_t *term_col, uint8_t *term_row)
{
  // Keep placing characters until we hit the null-terminating character ('\0')
  for (size_t i = 0; str[i] != '\0'; i ++)
  {
    terminal_putc(str[i], vga_buffer, VGA_COLS, VGA_ROWS, term_color, term_col,
      term_row);
  }
}
// -------------------------------------------------------------------------- //

