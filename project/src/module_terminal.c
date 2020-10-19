// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_terminal.h"
#include "module_kernel.h"
#include "module_base.h"
// -------------------------------------------------------------------------- //
module_terminal_vga *module_terminal_vga_instance = NULL;
void module_terminal_global_update_cursor();
// -------------------------------------------------------------------------- //
module_terminal_vga module_terminal_vga_create()
{
  module_terminal_vga t;
  t.buffer = (uint16_t*)(0xB8000); // = dec: 753664
  t.total_columns = 80;
  t.total_rows = 25;
  t.column_current = 0;
  t.row_current = 0;
  t.color = 0x0F;
  t.should_show_cursor = 0;
  return t;
}
// -------------------------------------------------------------------------- //
void module_terminal_global_init(const uint8_t should_show_cursor)
{
  module_terminal_vga_instance->should_show_cursor = should_show_cursor;
  if(module_terminal_vga_instance->should_show_cursor == 1)
  {
    module_terminal_global_enable_cursor();
    module_terminal_global_enable_cursor();
  }
  module_terminal_init(module_terminal_vga_instance);
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
void module_terminal_global_print_char(const char c)
{
  module_terminal_print_char(c, module_terminal_vga_instance);
  module_terminal_global_update_cursor();
}
// -------------------------------------------------------------------------- //
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
//    t->row_current = 0;
    t->row_current = t->total_rows-1;
    const size_t index = (t->total_columns * t->row_current)
      + t->column_current;
    module_kernel_memcpy(
      (uint8_t*)(t->buffer + t->total_columns),
      (uint8_t*)(t->buffer),
      2* (t->total_rows -1) * t->total_columns); // 2* because 2bytes/character 
    for(size_t i=0; i < t->total_columns; i++)
    {
      (t->buffer)[index+i] = ((uint16_t)(t->color) << 8) | ' ';
    }
  }
}
// -------------------------------------------------------------------------- //
void module_terminal_global_print_c_string(const char* str)
{
  module_terminal_print_c_string(str, module_terminal_vga_instance);
  module_terminal_global_update_cursor();
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
void module_terminal_global_print_buffer_hex_bytes(const uint8_t * const base,
  const size_t count)
{
//  module_terminal_global_print_c_string("printing b=");
//  module_terminal_global_print_uint64((uint32_t)(base));
//  module_terminal_global_print_c_string(" , len=");
//  module_terminal_global_print_uint64(count);
//  module_terminal_global_print_c_string("\n");

  char buffer[20+1];
  size_t l = 0;
  for (size_t i=0; i<count; i++)
  {
    l = module_base_uint64_to_ascii_base16(*(base + i), buffer);
    if(l == 1)
    {
      buffer[2] = '\0';
      buffer[1] = buffer[0];
      buffer[0] = '0';
    } else {
      buffer[l] = '\0';
    }
    module_terminal_global_print_c_string(buffer);
    if(i < count-1)
    {
      module_terminal_global_print_c_string(" ");
    }
  }
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void module_terminal_global_print_buffer_bytes(const uint8_t * const base,
  const size_t count)
{
//  module_terminal_global_print_c_string("printing b=");
//  module_terminal_global_print_uint64((uint32_t)(base));
//  module_terminal_global_print_c_string(" , len=");
//  module_terminal_global_print_uint64(count);
//  module_terminal_global_print_c_string("\n");

  for (size_t i=0; i<count; i++)
  {
    module_terminal_global_print_char(*( const char*)(base + i));
  }
}
// -------------------------------------------------------------------------- //
void module_terminal_global_print_uint8(const uint8_t i)
{
  module_terminal_print_uint8(i, module_terminal_vga_instance);
  module_terminal_global_update_cursor();
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
void module_terminal_global_print_uint64(const uint64_t i)
{
  module_terminal_print_uint64(i, module_terminal_vga_instance);
  module_terminal_global_update_cursor();
}
// -------------------------------------------------------------------------- //
void module_terminal_print_uint64(const uint64_t i, module_terminal_vga *t)
{
  char buffer[21];
  const size_t l = module_base_uint64_to_ascii_base10(i, buffer);
  buffer[l] = '\0';
  module_terminal_print_c_string(buffer, t);
}
// -------------------------------------------------------------------------- //
void module_terminal_global_print_hex_uint64(const uint64_t i)
{
  module_terminal_print_hex_uint64(i, module_terminal_vga_instance);
  module_terminal_global_update_cursor();
}
// -------------------------------------------------------------------------- //
void module_terminal_print_hex_uint64(const uint64_t i, module_terminal_vga *t)
{
  char buffer[20+1+2];
  buffer[0] = '0';
  buffer[1] = 'x';
  const size_t l = module_base_uint64_to_ascii_base16(i, buffer+2);
  buffer[2+l] = '\0';
  module_terminal_print_c_string(buffer, t);
}
// -------------------------------------------------------------------------- //
void module_terminal_global_print_binary_uint64(const uint64_t i)
{
  module_terminal_print_binary_uint64(i, module_terminal_vga_instance);
  module_terminal_global_update_cursor();
}
// -------------------------------------------------------------------------- //
void module_terminal_print_binary_uint64(const uint64_t i,
  module_terminal_vga *t)
{
  char buffer[64+1+2];
  buffer[0] = '0';
  buffer[1] = 'b';
  const size_t l = module_base_uint64_to_ascii_base2(i, buffer+2);
  buffer[l+2] = '\0';
  module_terminal_print_c_string(buffer, t);
}
// -------------------------------------------------------------------------- //
// Cursor
// -------------------------------------------------------------------------- //
void module_terminal_global_enable_cursor()
{
//  module_kernel_out_8(0x3D4, 0x0A);
//  module_kernel_out_8(0x3D5,
//    (module_kernel_in_8(0x3D5) & 0xC0) | cursor_start);
//  module_kernel_out_8(0x3D4, 0x0B);
//  module_kernel_out_8(0x3D5, (module_kernel_in_8(0x3D5) & 0xE0) | cursor_end);

  module_kernel_out_8(0x3D4, 0x0A);
  module_kernel_out_8(0x3D5, (module_kernel_in_8(0x3D5) & 0xC0));
  module_kernel_out_8(0x3D4, 0x0B);
  // 15 = scanline size of cursor
  module_kernel_out_8(0x3D5, (module_kernel_in_8(0x3E0) & 0xE0) | 15);
}
// -------------------------------------------------------------------------- //
void module_terminal_global_disable_cursor()
{
  module_kernel_out_8(0x3D4, 0x0A);
  module_kernel_out_8(0x3D5, 0x20);
}
// -------------------------------------------------------------------------- //
void module_terminal_global_update_cursor()
{
  const size_t index = (module_terminal_vga_instance->total_columns
    * module_terminal_vga_instance->row_current)
    + module_terminal_vga_instance->column_current;
  module_kernel_out_8(0x3D4, 0x0F);
  module_kernel_out_8(0x3D5, (uint8_t) (index & 0xFF));
  module_kernel_out_8(0x3D4, 0x0E);
  module_kernel_out_8(0x3D5, (uint8_t) ((index >> 8) & 0xFF));
}
// -------------------------------------------------------------------------- //

