// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_terminal.h"
#include "module_base.h"
// -------------------------------------------------------------------------- //
module_terminal_vga module_terminal_vga_create()
{
  module_terminal_vga t;
  t.buffer = (uint16_t*)(0xB8000);
  t.total_columns = 80;
  t.total_rows = 25;
  t.column_current = 0;
  t.row_current = 0;
  t.color = 0x0F;
  return t;
}
// -------------------------------------------------------------------------- //
void module_terminal_init(module_terminal_vga *t)
{
  // Clear the textmode buffer
  for (uint8_t col = 0; col < t->total_columns; col ++)
  {
    for (uint8_t row = 0; row < t->total_rows; row ++)
    {
      // The VGA textmode buffer has size (VGA_COLS * VGA_total_rows).
      // Given this, we find an index into the buffer for our character
      const size_t index = (t->total_columns * row) + col;
      // Entries in the VGA buffer take the binary form BBBBFFFFCCCCCCCC, where:
      // - B is the background color
      // - F is the foreground color
      // - C is the ASCII character
      // Set the character to blank (a space character)
      (t->buffer)[index] = ((uint16_t)(t->color) << 8) | ' ';
    }
  }
}
// -------------------------------------------------------------------------- //
// This function places a single character onto the screen
void module_terminal_print_char(const char c, module_terminal_vga *t)
{
  // Remember - we don't want to display ALL characters!
  switch (c)
  {
    case '\n':
    { // Newline characters should return the column to 0, and increment the row
      t->column_current = 0;
      t->row_current ++;
      break;
    }

    default:
    { // Normal characters just get displayed and then increment the column
      // Like before, calculate the buffer index
      const size_t index = (t->total_columns * t->row_current)
        + t->column_current;
      (t->buffer)[index] = ((uint16_t)(t->color) << 8) | c;
      t->column_current ++;
      break;
    }
  }
  // What happens if we get past the last column? We need to reset the column to
  // 0, and increment the row to get to a new line
  if (t->column_current >= t->total_columns)
  {
    t->column_current = 0;
    t->row_current ++;
  }
  // What happens if we get past the last row? We need to reset both column and
  // row to 0 in order to loop back to the top of the screen
  if (t->row_current >= t->total_rows)
  {
    t->column_current = 0;
    t->row_current = 0;
  }
}
// -------------------------------------------------------------------------- //
void module_terminal_print_c_string(const char* str, module_terminal_vga *t)
{
  // Keep placing characters until we hit the null-terminating character ('\0')
  for (size_t i = 0; str[i] != '\0'; i ++)
  {
    module_terminal_print_char(str[i], t);
  }
}
// -------------------------------------------------------------------------- //
void module_terminal_print_uint8(const uint8_t i, module_terminal_vga *t)
{
  if(i < 10)
  {
    module_terminal_print_char((char)(48+i), t);
  }
  else if(10 <= i && i < 100)
  {
    module_terminal_print_char((char)(48+i/10), t);
    module_terminal_print_char((char)(48+(i%10)), t);
  }
  else
  {
    module_terminal_print_char((char)(48+i/100), t);
    module_terminal_print_char((char)(48+((i/10)%10)), t);
    module_terminal_print_char((char)(48+(i%10)), t);
  }
}
// -------------------------------------------------------------------------- //
void module_terminal_print_uint64(const uint64_t i, module_terminal_vga *t)
{
  char buffer[21];
  const size_t l = uint64_to_ascii_base10(i, buffer);
  buffer[l] = '\0';
  module_terminal_print_c_string(buffer, t);
}
// -------------------------------------------------------------------------- //

