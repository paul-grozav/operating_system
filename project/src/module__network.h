// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK_H
#define MODULE__NETWORK_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
#include "module__network__data.h"
// -------------------------------------------------------------------------- //
//! All fields that describe a PCI device.
typedef struct module__network__struct_ethernet_interface
{
  /**
   * hardware address of this ethernet interface. Populated by driver when the
   * device is initialized.
   */
  module__network__data__mac_address mac_address;

  /**
   * Pointer to the next ethernet interface, making this a linked list.
   */
  struct module__network__struct_ethernet_interface * next_interface;
} module__network__ethernet_interface;
// -------------------------------------------------------------------------- //
/**
 * First node in a linked list of PCI device infos.
 */
extern module__network__ethernet_interface *
  module__network__ethernet_interfaces;
// -------------------------------------------------------------------------- //
//! Populate ethernet interface instance with default values
void module__network__ethernet_interface_init(
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
//! Populate global linked list of ethernet interfaces
void module__network__init_ethernet_interfaces();
// -------------------------------------------------------------------------- //
//! Free pointers from global linked list of ethernet interfaces
void module__network__free_ethernet_interfaces();
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//! Run short network test
void module__network__test();
void module__network__print_mac(
  const module__network__data__mac_address * const ma);
void module__network__print_ip(const uint32_t ip);
void module__network__process_ethernet_packet(
  const module__network__data__packet * const p);
void module__network__ip_checksum(module__network__data__packet *p);
void module__network__test2();
void module__network__queue__process();
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

