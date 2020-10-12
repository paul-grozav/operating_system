// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // NULL
#include "module__network__ethernet_interface.h"
#include "module__network.h"
#include "module_kernel.h"
#include "module_terminal.h"
#include "module_heap.h"
#include "module__driver__rtl8139.h"
// -------------------------------------------------------------------------- //
module__network__ethernet_interface *
  module__network__ethernet_interface__list = NULL;
// -------------------------------------------------------------------------- //
void module__network__ethernet_interface_init(
  module__network__ethernet_interface * const i)
{
  i->pci_device_info = NULL;
  i->driver = NULL;
  i->mac_address = module__network__data__mac_address__zero_mac;
  i->ipq_size = 100; // default
  i->incoming_packets_queue = NULL;
  i->incoming_packets_queue = malloc(i->ipq_size
    * sizeof(module__network__data__packet *));
  i->ipq_index = 0;
  i->next_interface = NULL;
}
// -------------------------------------------------------------------------- //
void module__network__ethernet_interface__init_all()
{
  module_terminal_global_print_c_string("Detecting ethernet interfaces.\n");
  if(module__pci__devices == NULL)
  {
    module_terminal_global_print_c_string("PCI Device List is EMPTY ?!");
    return;
  }
  // if here, then not empty ...

  // iterate to end of list
  const module__pci__device_info * i = module__pci__devices;
  do
  {
    if(
      i->class_code == 0x02 // Network controller
      && i->subclass_code == 0x00 // Ethernet controller
    )
    {
      if(i->vendor_id == 0x10ec && i->device_id == 0x8139)
      {
        // Realtek RTL8139 - https://wiki.osdev.org/RTL8139
        module_terminal_global_print_c_string("Initializing ethernet controller"
          " with PCI address: bus=");
        module_terminal_global_print_uint64(i->bus);
        module_terminal_global_print_c_string(", slot=");
        module_terminal_global_print_uint64(i->slot);
        module_terminal_global_print_c_string(", function=");
        module_terminal_global_print_uint64(i->function);
        module_terminal_global_print_c_string(", creating ethernet interface=");
        module__network__ethernet_interface * ei =
          malloc(sizeof(module__network__ethernet_interface));
        module_terminal_global_print_hex_uint64((uint32_t)(ei));
        module_terminal_global_print_c_string(" ...\n");
        module__network__ethernet_interface__list = ei;
        module__network__ethernet_interface_init(ei);
        ei->pci_device_info = i;
        module__driver__rtl8139__driver_init(i->bus, i->slot, i->function,
          &(ei->mac_address), &(ei->driver), ei);
        module__network__data__print_mac(&(ei->mac_address));
        module_terminal_global_print_c_string("\n");
      }
      else
      {
        module_terminal_global_print_c_string("No network driver for this PCI"
          " ethernet controller:\n");
        module__pci__print_device_info(i);
      }
    }
    else
    {
      // ignore PCI devices that are not ethernet controllers
//      module_terminal_global_print_c_string("NOT ETH: class=");
//      module_terminal_global_print_hex_uint64(i->class_code);
//      module_terminal_global_print_c_string("\n");
    }

    // Move to next device
    i = i->next_device;
  }
  while(i != NULL);
}
// -------------------------------------------------------------------------- //
void module__network__ethernet_interface__free_all()
{
  // iterate to end of list
  const module__network__ethernet_interface * i =
    module__network__ethernet_interface__list;
  module__network__ethernet_interface__list = NULL;
  const module__network__ethernet_interface * next = NULL;
  while(i != NULL)
  {
    next = i->next_interface;
    module_terminal_global_print_c_string("Freeing Ethernet interface ptr=");
    module_terminal_global_print_hex_uint64((uint32_t)(i));
    module_terminal_global_print_c_string(" from list.\n");
    // should i shut down the device in any way? maybe free something?
    free(i->incoming_packets_queue);
    free(i);
    // Move to next device
    i = next;
  }
}
// -------------------------------------------------------------------------- //
bool module__network__ethernet_interface__add_packet_to_incoming_queue(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const i)
{
  if(i->ipq_index == i->ipq_size)
  {
    module_terminal_global_print_c_string("Interface incoming queue is full");
    // queue is full
    return false;
  }
//  module_terminal_global_print_c_string("add on idx=");
//  module_terminal_global_print_uint64(i->ipq_index);
//  module_terminal_global_print_c_string(" at addr=");
//  module_terminal_global_print_hex_uint64(
//    (uint32_t)(&(i->incoming_packets_queue[i->ipq_index])));
//  module_terminal_global_print_c_string(" on iface=");
//  module_terminal_global_print_hex_uint64((uint32_t)(i));
//  module_terminal_global_print_c_string("\n");
  i->incoming_packets_queue[i->ipq_index] = p;
  i->ipq_index += 1;
  return true;
}
// -------------------------------------------------------------------------- //
const module__network__data__packet *
  module__network__ethernet_interface__get_packet_from_incoming_queue(
  module__network__ethernet_interface * const i)
{
//  module_terminal_global_print_c_string("get packet from idx=0");
//  module_terminal_global_print_c_string(" at addr=");
//  module_terminal_global_print_hex_uint64(
//    (uint64_t)(&(i->incoming_packets_queue[0])));
//  module_terminal_global_print_c_string(" when idx=");
//  module_terminal_global_print_uint64(i->ipq_index);
//  module_terminal_global_print_c_string("\n");
  const module__network__data__packet * const r = i->incoming_packets_queue[0];
  i->ipq_index -= 1;
  module_kernel_memcpy(&(i->incoming_packets_queue[1]),
    &(i->incoming_packets_queue[0]),
    i->ipq_index * sizeof(module__network__data__packet *));
  i->incoming_packets_queue[i->ipq_index] = NULL;
  return r;
}
// -------------------------------------------------------------------------- //
void module__network__ethernet_interface__send_packet(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const i)
{
  if(
    i->pci_device_info->class_code == 0x02 // Network controller
    && i->pci_device_info->subclass_code == 0x00 // Ethernet controller
  )
  {
    if(i->pci_device_info->vendor_id == 0x10ec
      && i->pci_device_info->device_id == 0x8139)
    {
      module__driver__rtl8139__send_packet(p, i->driver);
    }
    else
    {
//      module_terminal_global_print_c_string("ERROR: No send handler for this"
//        " driver!\n");
//      module_pci_print_device_info(i->pci_device_info);
    }
  }
  else
  {
    // this PCI device should be a NIC, since it's associated with an ethernet
    // controller.
    // ignore PCI devices that are not ethernet controllers
//      module_terminal_global_print_c_string("NOT ETH: class=");
//      module_terminal_global_print_hex_uint64(i->class_code);
//      module_terminal_global_print_c_string("\n");
  }
}
// -------------------------------------------------------------------------- //
