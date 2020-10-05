// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__ETHERNET_INTERFACE_H
#define MODULE__NETWORK__ETHERNET_INTERFACE_H
// -------------------------------------------------------------------------- //
//#include <stdint.h> // uintX
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
  module__network__ethernet_interface_list;
// -------------------------------------------------------------------------- //
//! Populate ethernet interface instance with default values
void module__network__ethernet_interface__init(
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
//! Populate global linked list of ethernet interfaces
void module__network__ethernet_interface_init_all();
// -------------------------------------------------------------------------- //
//! Free pointers from global linked list of ethernet interfaces
void module__network__ethernet_interface_free_all();
// -------------------------------------------------------------------------- //
//! Populate ethernet interface instance with default values
void module__network__ethernet_interface__send_packet(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const i);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
