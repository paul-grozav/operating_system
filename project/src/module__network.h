// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK_H
#define MODULE__NETWORK_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
#include "module_pci.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//! Run short network test
void module__network__test();
void module__network__print_mac(
  const module__network__data__mac_address * const ma);
void module__network__print_ip(const uint32_t ip);
void module__network__process_ethernet_packet(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const interface);
void module__network__ip_checksum(module__network__data__packet *p);
void module__network__test2();
void module__network__queue__process();
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
