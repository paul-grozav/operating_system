// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE_KEYBOARD_H
#define MODULE_KEYBOARD_H
// -------------------------------------------------------------------------- //
//! Register interrupt handler and enable keyboard interrupt
void module_keyboard_enable();

//! Wait and return when key is pressed
void module_keyboard_wait_keypress();
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
