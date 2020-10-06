// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__ETHERNET_INTERFACE_H
#define MODULE__NETWORK__ETHERNET_INTERFACE_H
// -------------------------------------------------------------------------- //
//#include <stdint.h> // uintX
#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include "module__network__data.h"
#include "module__pci.h"
// -------------------------------------------------------------------------- //
//! All fields that describe an ethernet interface.
typedef struct module__network__struct_ethernet_interface
{
  //! PCI device info, that is used to talk to this eth interface / hardware
  const module__pci__device_info * pci_device_info;

  //! The actual driver used, is based on the vendor and device id in PCI info.
  void * driver;

  /**
   * hardware address of this ethernet interface. Populated by driver when the
   * device is initialized.
   */
  module__network__data__mac_address mac_address;

  //! Size of incoming packets queue
  size_t ipq_size;

  //! Incoming packets queue
  const module__network__data__packet ** incoming_packets_queue;

  /** Incoming packets queue index. Points to next position where we should
   * place the next packet.
   */
  size_t ipq_index;

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
  module__network__ethernet_interface__list;
// -------------------------------------------------------------------------- //
//! Populate ethernet interface instance with default values
void module__network__ethernet_interface__init(
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
//! Populate global linked list of ethernet interfaces
void module__network__ethernet_interface__init_all();
// -------------------------------------------------------------------------- //
//! Free pointers from global linked list of ethernet interfaces
void module__network__ethernet_interface__free_all();
// -------------------------------------------------------------------------- //
//! Add packet to incoming packets queue
bool module__network__ethernet_interface__add_packet_to_incoming_queue(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
//! Get packet from incoming packets queue
const module__network__data__packet *
  module__network__ethernet_interface__get_packet_from_incoming_queue(
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
//! Populate ethernet interface instance with default values
void module__network__ethernet_interface__send_packet(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
