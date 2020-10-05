// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__DRIVER__RTL8139_H
#define MODULE__DRIVER__RTL8139_H
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include "module_interrupt.h"
#include "module__network__data.h"
// -------------------------------------------------------------------------- //
//! All fields that describe a RTL8139 driver instance.
typedef struct module__driver__rtl8139__struct_instance
{
  //! Base address used for IO communication with the hardware / device.
  uint16_t iobase;

  //module_heap_heap_bm nic_heap;

  //! size of receiver buffer in bytes
  size_t rx_buff_size;

  //! A pointer to the receive buffer allocated on heap
  uint8_t *rx_buffer;

  //! Receive index in buffer
  size_t rx_index;

  /**
   * RTL8139 has 4 transmission buffers( 1, 2, 3, 4), we have to use them in
   * this order.
   */
  uint32_t tx_slot;

  /**
   * Pointer to the ethernet interface associated with this driver instance.
   * When you get a IRQ about having a packet on one of the driver instances,
   * you cand detect the driver instance, and it would be nice to have a pointer
   * to the ethernet interface, instead of searching it for every received
   * packet.
   */
  void * ethernet_interface;

  //! Pointer to the next driver, making this a linked list
  struct module__driver__rtl8139__struct_instance * next_driver;
} module__driver__rtl8139__instance;
// -------------------------------------------------------------------------- //
/**
 * First node in a linked list of driver instances.
 */
extern module__driver__rtl8139__instance * module__driver__rtl8139__instances;
// -------------------------------------------------------------------------- //
//! Add driver instance to global linked list
void module__driver__rtl8139__global_list_add_driver(
  module__driver__rtl8139__instance * driver);
// -------------------------------------------------------------------------- //
//! Handle an interrupt for one (not yet known) of the driver instances
void module__driver__rtl8139__generic_interrupt_handler(
  module_interrupt_registers_t x);
// -------------------------------------------------------------------------- //
//! Handle an interrupt for the given driver instance
void module__driver__rtl8139__interrupt_handler(
  module__driver__rtl8139__instance * driver, const uint16_t flags);
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__send_packet(
  const module__network__data__packet * const p,
  module__driver__rtl8139__instance * driver);
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__driver_init(const uint8_t bus, const uint8_t slot,
  const uint8_t function,
  module__network__data__mac_address * const mac_address, void ** driver_ptr,
  void * ethernet_interface);
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__driver_free(
  module__driver__rtl8139__instance * driver);
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__driver_free_all();
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

