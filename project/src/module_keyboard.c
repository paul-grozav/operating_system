// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include "module_keyboard.h"
#include "module_kernel.h"
#include "module_interrupt.h"
#include "module_terminal.h"
// -------------------------------------------------------------------------- //
// the byte we get when pressing a key, contains 1 bit that tells us if the key
// was pressed or released. Thus, remaining 7 bits to identify the key that was
// pressed/released. 127 values can be expressed with 7 bits.
//
// The purpose of this mapping is to convert the key code we get into an ascii
// character. To do this, you
char module_keyboard_mapping[127] =
{
 '\0', // 0 - UNKNOWN KEY
 ' ', // 1 - ESCAPE
 '1', // 2
 '2', // 3
 '3', // 4
 '4', // 5
 '5', // 6
 '6', // 7
 '7', // 8
 '8', // 9
 '9', // 10
 '0', // 11
 '-', // 12
 '=', // 13
 ' ', // 14 - BACKSPACE
 ' ', // 15 - TAB
 'q', // 16
 'w', // 17
 'e', // 18
 'r', // 19
 't', // 20
 'y', // 21
 'u', // 22
 'i', // 23
 'o', // 24
 'p', // 25
 '[', // 26
 ']', // 27
 ' ', // 28 - ENTER
 ' ', // 29 - CTRL (LEFT and RIGHT)
 'a', // 30
 's', // 31
 'd', // 32
 'f', // 33
 'g', // 34
 'h', // 35
 'j', // 36
 'k', // 37
 'l', // 38
 ';', // 39
 '\'', // 40
 '`', // 41 - back tick, next to 1(one)
 ' ', // 42 - SHIFT Left
 '\\', // 43
 'z', // 44
 'x', // 45
 'c', // 46
 'v', // 47
 'b', // 48
 'n', // 49
 'm', // 50
 ',', // 51
 '.', // 52
 '/', // 53
 ' ', // 54 - SHIFT Right
 ' ', // 55 - NUMPAD ASTERISK
 ' ', // 56 - ALT (LEFT and RIGHT)
 ' ', // 57 - SPACE - The actual space key on your keyboard
 ' ', // 58 - CAPS LOCK
 ' ', // 59 - F1
 ' ', // 60 - F2
 ' ', // 61 - F3
 ' ', // 62 - F4
 ' ', // 63 - F5
 ' ', // 64 - F6
 ' ', // 65 - F7
 ' ', // 66 - F8
 ' ', // 67 - F9
 ' ', // 68 - F10
 ' ', // 69 - NUM LOCK
 ' ', // 70 - SCROLL LOCK
 ' ', // 71 - HOME
 ' ', // 72 - UP ARROW
 ' ', // 73 - PAGE UP
 ' ', // 74 - NUMPAD MINUS sign
 ' ', // 75 - LEFT ARROW
 ' ', // 76 - NUMPAD key 5
 ' ', // 77 - RIGHT ARROW
 ' ', // 78 - NUMPAD PLUS sign
 ' ', // 79 - END
 ' ', // 80 - DOWN ARROW
 ' ', // 81 - PAGE DOWN
 ' ', // 82 - INSERT
 ' ', // 83 - DELETE
 '\0', // 84 - UNKNOWN KEY
 '\0', // 85 - UNKNOWN KEY
 '\0', // 86 - UNKNOWN KEY
 ' ', // 87 - F11
 ' ', // 88 - F12
 '\0', // 89 - UNKNOWN KEY
 '\0', // 90 - UNKNOWN KEY
 ' ', // 91 - WINDOWS KEY (LEFT)
 ' ', // 92 - WINDOWS KEY (RIGHT)
 ' ', // 93 - CONTEXT MENU KEY
 '\0', // 94 - UNKNOWN KEY
 '\0', // 95 - UNKNOWN KEY
 '\0', // 96 - UNKNOWN KEY
 '\0', // 97 - UNKNOWN KEY
 '\0', // 98 - UNKNOWN KEY
 '\0', // 99 - UNKNOWN KEY
 '\0', // 100 - UNKNOWN KEY
 '\0', // 101 - UNKNOWN KEY
 '\0', // 102 - UNKNOWN KEY
 '\0', // 103 - UNKNOWN KEY
 '\0', // 104 - UNKNOWN KEY
 '\0', // 105 - UNKNOWN KEY
 '\0', // 106 - UNKNOWN KEY
 '\0', // 107 - UNKNOWN KEY
 '\0', // 108 - UNKNOWN KEY
 '\0', // 109 - UNKNOWN KEY
 '\0', // 110 - UNKNOWN KEY
 '\0', // 111 - UNKNOWN KEY
 '\0', // 112 - UNKNOWN KEY
 '\0', // 113 - UNKNOWN KEY
 '\0', // 114 - UNKNOWN KEY
 '\0', // 115 - UNKNOWN KEY
 '\0', // 116 - UNKNOWN KEY
 '\0', // 117 - UNKNOWN KEY
 '\0', // 118 - UNKNOWN KEY
 '\0', // 119 - UNKNOWN KEY
 '\0', // 120 - UNKNOWN KEY
 '\0', // 121 - UNKNOWN KEY
 '\0', // 122 - UNKNOWN KEY
 '\0', // 123 - UNKNOWN KEY
 '\0', // 124 - UNKNOWN KEY
 '\0', // 125 - UNKNOWN KEY
 '\0', // 126 - UNKNOWN KEY
};
// -------------------------------------------------------------------------- //
//const uint8_t module_keyboard_mapping_key_??? = 0;
const uint8_t module_keyboard_mapping_key_ESCAPE = 1;
const uint8_t module_keyboard_mapping_key_1 = 2;
const uint8_t module_keyboard_mapping_key_2 = 3;
const uint8_t module_keyboard_mapping_key_3 = 4;
const uint8_t module_keyboard_mapping_key_4 = 5;
const uint8_t module_keyboard_mapping_key_5 = 6;
const uint8_t module_keyboard_mapping_key_6 = 7;
const uint8_t module_keyboard_mapping_key_7 = 8;
const uint8_t module_keyboard_mapping_key_8 = 9;
const uint8_t module_keyboard_mapping_key_9 = 10;
const uint8_t module_keyboard_mapping_key_0 = 11;
const uint8_t module_keyboard_mapping_key_MINUS = 12;
const uint8_t module_keyboard_mapping_key_EQUAL = 13;
const uint8_t module_keyboard_mapping_key_BACKSPACE = 14;
const uint8_t module_keyboard_mapping_key_TAB = 15;
const uint8_t module_keyboard_mapping_key_Q = 16;
const uint8_t module_keyboard_mapping_key_W = 17;
const uint8_t module_keyboard_mapping_key_E = 18;
const uint8_t module_keyboard_mapping_key_R = 19;
const uint8_t module_keyboard_mapping_key_T = 20;
const uint8_t module_keyboard_mapping_key_Y = 21;
const uint8_t module_keyboard_mapping_key_U = 22;
const uint8_t module_keyboard_mapping_key_I = 23;
const uint8_t module_keyboard_mapping_key_O = 24;
const uint8_t module_keyboard_mapping_key_P = 25;
const uint8_t module_keyboard_mapping_key_SQUARE_BRACKET_LEFT = 26;
const uint8_t module_keyboard_mapping_key_SQUARE_BRACKET_RIGHT = 27;
const uint8_t module_keyboard_mapping_key_ENTER = 28;
const uint8_t module_keyboard_mapping_key_CONTROL = 29;
const uint8_t module_keyboard_mapping_key_A = 30;
const uint8_t module_keyboard_mapping_key_S = 31;
const uint8_t module_keyboard_mapping_key_D = 32;
const uint8_t module_keyboard_mapping_key_F = 33;
const uint8_t module_keyboard_mapping_key_G = 34;
const uint8_t module_keyboard_mapping_key_H = 35;
const uint8_t module_keyboard_mapping_key_J = 36;
const uint8_t module_keyboard_mapping_key_K = 37;
const uint8_t module_keyboard_mapping_key_L = 38;
const uint8_t module_keyboard_mapping_key_SEMICOLON = 39;
const uint8_t module_keyboard_mapping_key_APOSTROPHE = 40;
const uint8_t module_keyboard_mapping_key_BACK_TICK = 41;
const uint8_t module_keyboard_mapping_key_SHIFT_LEFT = 42;
const uint8_t module_keyboard_mapping_key_BACKSLASH = 43;
const uint8_t module_keyboard_mapping_key_Z = 44;
const uint8_t module_keyboard_mapping_key_X = 45;
const uint8_t module_keyboard_mapping_key_C = 46;
const uint8_t module_keyboard_mapping_key_V = 47;
const uint8_t module_keyboard_mapping_key_B = 48;
const uint8_t module_keyboard_mapping_key_N = 49;
const uint8_t module_keyboard_mapping_key_M = 50;
const uint8_t module_keyboard_mapping_key_COMMA = 51;
const uint8_t module_keyboard_mapping_key_PERIOD = 52;
const uint8_t module_keyboard_mapping_key_SLASH = 53;
const uint8_t module_keyboard_mapping_key_SHIFT_RIGHT = 54;
const uint8_t module_keyboard_mapping_key_NUMPAD_ASTERISK = 55;
const uint8_t module_keyboard_mapping_key_ALT = 56;
const uint8_t module_keyboard_mapping_key_SPACE = 57;
const uint8_t module_keyboard_mapping_key_CAPS_LOCK = 58;
const uint8_t module_keyboard_mapping_key_F1 = 59;
const uint8_t module_keyboard_mapping_key_F2 = 60;
const uint8_t module_keyboard_mapping_key_F3 = 61;
const uint8_t module_keyboard_mapping_key_F4 = 62;
const uint8_t module_keyboard_mapping_key_F5 = 63;
const uint8_t module_keyboard_mapping_key_F6 = 64;
const uint8_t module_keyboard_mapping_key_F7 = 65;
const uint8_t module_keyboard_mapping_key_F8 = 66;
const uint8_t module_keyboard_mapping_key_F9 = 67;
const uint8_t module_keyboard_mapping_key_F10 = 68;
const uint8_t module_keyboard_mapping_key_NUM_LOCK = 69;
const uint8_t module_keyboard_mapping_key_SCROLL_LOCK = 70;
const uint8_t module_keyboard_mapping_key_HOME = 71;
const uint8_t module_keyboard_mapping_key_ARROW_UP = 72;
const uint8_t module_keyboard_mapping_key_PAGE_UP = 73;
const uint8_t module_keyboard_mapping_key_NUMPAD_MINUS = 74;
const uint8_t module_keyboard_mapping_key_ARROW_LEFT = 75;
const uint8_t module_keyboard_mapping_key_NUMPAD_KEY_5 = 76;
const uint8_t module_keyboard_mapping_key_ARROW_RIGHT = 77;
const uint8_t module_keyboard_mapping_key_NUMPAD_PLUS = 78;
const uint8_t module_keyboard_mapping_key_END = 79;
const uint8_t module_keyboard_mapping_key_ARROW_DOWN = 80;
const uint8_t module_keyboard_mapping_key_PAGE_DOWN = 81;
const uint8_t module_keyboard_mapping_key_INSERT = 82;
const uint8_t module_keyboard_mapping_key_DELETE = 83;
//const uint8_t module_keyboard_mapping_key_??? = 84;
//const uint8_t module_keyboard_mapping_key_??? = 85;
//const uint8_t module_keyboard_mapping_key_??? = 86;
const uint8_t module_keyboard_mapping_key_F11 = 87;
const uint8_t module_keyboard_mapping_key_F12 = 88;
//const uint8_t module_keyboard_mapping_key_??? = 89;
//const uint8_t module_keyboard_mapping_key_??? = 90;
const uint8_t module_keyboard_mapping_key_WINDOWS_KEY_LEFT = 91;
const uint8_t module_keyboard_mapping_key_WINDOWS_KEY_RIGHT = 92;
const uint8_t module_keyboard_mapping_key_CONTEXT_MENU_KEY = 93;
//const uint8_t module_keyboard_mapping_key_??? = 94;
//const uint8_t module_keyboard_mapping_key_??? = 95;
//const uint8_t module_keyboard_mapping_key_??? = 96;
//const uint8_t module_keyboard_mapping_key_??? = 97;
//const uint8_t module_keyboard_mapping_key_??? = 98;
//const uint8_t module_keyboard_mapping_key_??? = 99;
//const uint8_t module_keyboard_mapping_key_??? = 100;
//const uint8_t module_keyboard_mapping_key_??? = 101;
//const uint8_t module_keyboard_mapping_key_??? = 102;
//const uint8_t module_keyboard_mapping_key_??? = 103;
//const uint8_t module_keyboard_mapping_key_??? = 104;
//const uint8_t module_keyboard_mapping_key_??? = 105;
//const uint8_t module_keyboard_mapping_key_??? = 106;
//const uint8_t module_keyboard_mapping_key_??? = 107;
//const uint8_t module_keyboard_mapping_key_??? = 108;
//const uint8_t module_keyboard_mapping_key_??? = 109;
//const uint8_t module_keyboard_mapping_key_??? = 110;
//const uint8_t module_keyboard_mapping_key_??? = 111;
//const uint8_t module_keyboard_mapping_key_??? = 112;
//const uint8_t module_keyboard_mapping_key_??? = 113;
//const uint8_t module_keyboard_mapping_key_??? = 114;
//const uint8_t module_keyboard_mapping_key_??? = 115;
//const uint8_t module_keyboard_mapping_key_??? = 116;
//const uint8_t module_keyboard_mapping_key_??? = 117;
//const uint8_t module_keyboard_mapping_key_??? = 118;
//const uint8_t module_keyboard_mapping_key_??? = 119;
//const uint8_t module_keyboard_mapping_key_??? = 120;
//const uint8_t module_keyboard_mapping_key_??? = 121;
//const uint8_t module_keyboard_mapping_key_??? = 122;
//const uint8_t module_keyboard_mapping_key_??? = 123;
//const uint8_t module_keyboard_mapping_key_??? = 124;
//const uint8_t module_keyboard_mapping_key_??? = 125;
//const uint8_t module_keyboard_mapping_key_??? = 126;
// -------------------------------------------------------------------------- //
volatile uint8_t ScanCode;
void module_keyboard_interrupt_handler(module_interrupt_registers_t x)
{
//  module_terminal_global_print_c_string("IRQ1 handler\n");
  module_kernel_out_8(0x20, 0x20);// Send End-of-interrupt
  ScanCode = module_kernel_in_8(0x60);
  // MSB 1000000 tells us if it was a key up or down
  // the other bits describe the key
  uint8_t was_pressed = 0;
  if((ScanCode & 128) != 128)
  {
    was_pressed = 1;
  }
  else
  {
//    ScanCode = ScanCode & ~(0<<7); // Remove bit - set to 0
    ScanCode = ScanCode - 128;
    return; // ignore key ups
  }
  // Print interrupt/key details
/*
  if(was_pressed == 1)
  {
    module_terminal_global_print_c_string("Pressed");
  }
  else
  {
    module_terminal_global_print_c_string("Released");
  }
  module_terminal_global_print_char(' ');
  module_terminal_global_print_uint64(ScanCode);
  module_terminal_global_print_char('=');
  module_terminal_global_print_uint64(ScanCode & ~(0<<7));
  module_terminal_global_print_char(' ');
  uint8_t c = module_keyboard_mapping[ScanCode];
  if( c == '\0' )
  {
    module_terminal_global_print_c_string("unknown key ");
    module_terminal_global_print_uint64(ScanCode);
  }
  module_terminal_global_print_c_string(" character '");
  module_terminal_global_print_char(c);
  module_terminal_global_print_c_string("'\n");
*/
//*
  const uint8_t c = module_keyboard_mapping[ScanCode];
  if( c == '\0' )
  {
    module_terminal_global_print_char('\n');
    module_terminal_global_print_c_string("unknown key: ");
    module_terminal_global_print_uint64(ScanCode);
    module_terminal_global_print_char('\n');
  }
  else if( ScanCode == module_keyboard_mapping_key_ENTER )
  {
    module_terminal_global_print_char('\n');
  }
  else
  {
    module_terminal_global_print_char(c);
  }
//*/
  module_interrupt_enable();
}
// -------------------------------------------------------------------------- //
void module_keyboard_enable()
{
  module_interrupt_register_interrupt_handler(32+1,
    module_keyboard_interrupt_handler);
  module_interrupt_enable_irq(32+1);
}
// -------------------------------------------------------------------------- //
void module_keyboard_wait_keypress()
{
  int key;
  while ( 1 )
  {

//    module_kernel_out_8(0x20, 0x20); // Send EOI
//    unsigned char c = module_kernel_in_8( 0x60 );
//    if((c & 128) == 128)
//      module_terminal_global_print_c_string("RELEASE\n");
//    else
//      module_terminal_global_print_c_string("PRESS\n");

    // wait for key
    while ((module_kernel_in_8(0x64) & 1) == 0);
    key = module_kernel_in_8( 0x60 ); // same as inb- use yours
    module_terminal_global_print_uint64(key);
    if ( key & 0x80 ) continue;
    if ( key != 0 ) return;
    else if ( key == 0 ) continue;
  }
}
// -------------------------------------------------------------------------- //

