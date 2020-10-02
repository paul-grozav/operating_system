// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__DRIVER__RTL8139_H
#define MODULE__DRIVER__RTL8139_H
// -------------------------------------------------------------------------- //
#include "module__network__data.h"
// -------------------------------------------------------------------------- //
//! Run short network test
void module__driver__rtl8139__send_packet(
  const module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__init_device(const uint8_t bus, const uint8_t slot,
  const uint8_t function,
  module__network__data__mac_address * const mac_address);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

