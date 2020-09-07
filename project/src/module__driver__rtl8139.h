// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__DRIVER__RTL8139_H
#define MODULE__DRIVER__RTL8139_H
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//! Run short network test
void module__driver__rtl8139__send_packet(
  const module__network__packet * const p);
void module__driver__rtl8139__test();
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

