// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK_H
#define MODULE__NETWORK_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
//#include "module__pci.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//! Run short network test
void module__network__test();
// -------------------------------------------------------------------------- //
void module__network__process_ethernet_packet(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const interface);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
