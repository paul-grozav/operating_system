// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// See : https://opensource.apple.com/source/boot/boot-83.2/i386/libsaio
// /legacy/PCI.h.auto.html
//
// Compile and run using:
// ~>docker exec -it os bash /mnt/run.sh &&
// qemu-system-i386 -cdrom ~/data/h313/network/docker/research/os/
// build_machine/fs/project/build/bootable.iso -boot d
// -netdev user,id=mynet0 -device rtl8139,netdev=mynet0
#include "module_pci.h"
#include "module_kernel.h"
#include "module_heap.h"
#include "module_terminal.h"
#include "module_base.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//#define OFFSET_DEVICEID 0x02
//#define OFFSET_VENDORID 0x00
//#define MULTI_FUNCTION 0x80
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
#define PCI_CONFIG 0xCF8
#define PCI_DATA 0xCFC
// config Offsets
//#define PCI_VENDOR_DEVICE 0x00
//#define PCI_CLASS_SUBCLASS 0x08
//#define PCI_BAR0 0x10
//#define PCI_BAR1 PCI_BAR0 + 4
//#define PCI_BAR2 PCI_BAR1 + 4
//#define PCI_BAR3 PCI_BAR2 + 4
//#define PCI_BAR4 PCI_BAR3 + 4
//#define PCI_BAR5 PCI_BAR4 + 4
// -------------------------------------------------------------------------- //
uint32_t module_pci_config_read(const uint8_t bus, const uint8_t slot,
  const uint8_t function, const uint8_t offset)
{
  uint32_t reg = 0x80000000;
  reg |= (bus & 0xff) << 16;
  reg |= (slot & 0x1f) << 11;
  reg |= (function & 0x7) << 8;
  reg |= (offset & 0xff) & 0xfc;
  module_kernel_out_32(PCI_CONFIG, reg);
  return module_kernel_in_32(PCI_DATA);
}
// -------------------------------------------------------------------------- //
void module_pci_config_write(const uint8_t bus, const uint8_t slot,
  const uint8_t function, const uint8_t offset, const uint32_t data)
{
  uint32_t reg = 0x80000000;
  reg |= (bus & 0xff) << 16;
  reg |= (slot & 0x1f) << 11;
  reg |= (function & 0x7) << 8;
  reg |= offset & 0xfc;
  module_kernel_out_32(PCI_CONFIG, reg);
  module_kernel_out_32(PCI_DATA, data);
}
// -------------------------------------------------------------------------- //
void module_pci_device_info_init(module_pci_device_info * const di)
{
  if(di == NULL)
  {
    return;
  }
  const uint16_t invalid_vendor_id = 0xffff;
  di->vendor_id = invalid_vendor_id;
  di->device_id = 0;
  di->is_multifunction_device = 0;
  di->function = 0;
  di->status = 0;
  di->command = 0;
  di->class_code = 0;
  di->subclass_code = 0;
  di->prog_if = 0;
  di->revision_id = 0;
  di->bist = 0;
  di->header_type = 0;
  di->latency_timer = 0;
  di->cache_line_size = 0;
  di->next_device = NULL;
}
// -------------------------------------------------------------------------- //
module_pci_device_info * module_pci_devices = NULL;
// -------------------------------------------------------------------------- //
void module_pci_device_get_class_name(const uint8_t class,
  const uint8_t subclass, char ** class_name, char ** subclass_name)
{
  // Based on: https://wiki.osdev.org/PCI#Class_Codes
  *subclass_name = "UNKNOWN"; // not all implemented
  if(class == 0x00)
  {
    *class_name = "Unclassified";
  }
  else if(class == 0x01)
  {
    *class_name = "Mass Storage Controller";
  }
  else if(class == 0x02)
  {
    *class_name = "Network Controller";
    if(subclass == 0x00)
    {
      *subclass_name = "Ethernet Controller";
    }
    else
    {
      *subclass_name = "UNKNOWN";
    }
  }
  else if(class == 0x03)
  {
    *class_name = "Display Controller";
    if(subclass == 0x00)
    {
      *subclass_name = "VGA Compatible Controller";
    }
    else
    {
      *subclass_name = "UNKNOWN";
    }
  }
  else if(class == 0x04)
  {
    *class_name = "Multimedia Controller";
  }
  else if(class == 0x05)
  {
    *class_name = "Memory Controller";
  }
  else if(class == 0x06)
  {
    *class_name = "Bridge Device";
    if(subclass == 0x00)
    {
      *subclass_name = "Host Bridge";
    }
    else if(subclass == 0x01)
    {
      *subclass_name = "ISA Bridge";
    }
    else
    {
      *subclass_name = "UNKNOWN";
    }
  }
  else if(class == 0x07)
  {
    *class_name = "Simple Communication Controller";
  }
  else if(class == 0x08)
  {
    *class_name = "Base System Peripheral";
  }
  else if(class == 0x09)
  {
    *class_name = "Input Device Controller";
  }
  else if(class == 0x0a)
  {
    *class_name = "Docking Station";
  }
  else if(class == 0x0b)
  {
    *class_name = "Processor";
  }
  else if(class == 0x0c)
  {
    *class_name = "Serial Bus Controller";
  }
  else if(class == 0x0d)
  {
    *class_name = "Wireless Controller";
  }
  else if(class == 0x0e)
  {
    *class_name = "Intelligent Controller";
  }
  else if(class == 0x0f)
  {
    *class_name = "Intelligent Controller";
  }
  else if(class == 0x10)
  {
    *class_name = "Encryption Controller";
  }
  else if(class == 0x11)
  {
    *class_name = "Signal Processing Controller";
  }
  else if(class == 0x12)
  {
    *class_name = "Processing Accelerator";
  }
  else if(class == 0x13)
  {
    *class_name = "Non-Essential Instrumentation";
  }
  else if(class >= 0x14 && class <= 0x3f)
  {
    *class_name = "(Reserved range 1)";
  }
  else if(class == 0x40)
  {
    *class_name = "Co-Processor";
  }
  else if(class >= 0x41 && class <= 0xfe)
  {
    *class_name = "(Reserved range 2)";
  }
  else if(class == 0xff)
  {
    *class_name = "Unassigned Class (Vendor specific)";
  }
  else
  {
    *class_name = "UNKNOWN";
  }
}
// -------------------------------------------------------------------------- //
void module_pci_print_device_info(const module_pci_device_info * const di)
{
  if(di == NULL)
  {
    module_terminal_global_print_c_string("NULL PCI device.\n\n");
    return;
  }
  char * class_name = NULL;
  char * subclass_name = NULL;
  module_pci_device_get_class_name(di->class_code, di->subclass_code,
    &class_name, &subclass_name);

  module_terminal_global_print_c_string("PCI device list_ptr=");
  module_terminal_global_print_hex_uint64((uint32_t)(di));
  module_terminal_global_print_c_string(" has the following info:");
  module_terminal_global_print_c_string("   bus=");
  module_terminal_global_print_uint8(di->bus);
  module_terminal_global_print_c_string("   slot=");
  module_terminal_global_print_uint64(di->slot);
  module_terminal_global_print_c_string("   vendor=");
  module_terminal_global_print_hex_uint64(di->vendor_id);
  module_terminal_global_print_c_string("   device=");
  module_terminal_global_print_hex_uint64(di->device_id);
  module_terminal_global_print_c_string("   class=");
  module_terminal_global_print_hex_uint64(di->class_code);
  module_terminal_global_print_c_string("(\"");
  module_terminal_global_print_c_string(class_name);
  module_terminal_global_print_c_string("\")");
  module_terminal_global_print_c_string("   subclass=");
  module_terminal_global_print_hex_uint64(di->subclass_code);
  module_terminal_global_print_c_string("(\"");
  module_terminal_global_print_c_string(subclass_name);
  module_terminal_global_print_c_string("\")");
  module_terminal_global_print_c_string("   is_multi_function=");
  module_terminal_global_print_uint8(di->is_multifunction_device);
  module_terminal_global_print_c_string("   function=");
  module_terminal_global_print_uint8(di->function);
  module_terminal_global_print_c_string("   command=");
  module_terminal_global_print_binary_uint64(di->command);
  module_terminal_global_print_c_string("   status=");
  module_terminal_global_print_uint64(di->status);
  module_terminal_global_print_c_string("   prog_if=");
  module_terminal_global_print_uint8(di->prog_if);
  module_terminal_global_print_c_string("   revision_id=");
  module_terminal_global_print_uint8(di->revision_id);
  module_terminal_global_print_c_string("   latency_timer=");
  module_terminal_global_print_uint8(di->latency_timer);
  module_terminal_global_print_c_string("   cache_line_size=");
  module_terminal_global_print_uint8(di->cache_line_size);
  module_terminal_global_print_c_string("   header_type=");
  module_terminal_global_print_binary_uint64(di->header_type);
  module_terminal_global_print_c_string("   BIST=");
  module_terminal_global_print_binary_uint64(di->bist);
  module_terminal_global_print_c_string("   BAR0=");
  module_terminal_global_print_hex_uint64(di->bar_0);
  module_terminal_global_print_c_string("   next_device=");
  module_terminal_global_print_hex_uint64((uint32_t)(di->next_device));
  module_terminal_global_print_c_string(" .\n\n");
}
// -------------------------------------------------------------------------- //
// add copy to list
void module_pci_add_device_copy_to_list(const module_pci_device_info * const di)
{
  // create copy on heap
  module_pci_device_info * di_node = (module_pci_device_info *)malloc(
    sizeof(module_pci_device_info));
  *di_node = *di;
  module_terminal_global_print_c_string("Adding PCI device ptr=");
  module_terminal_global_print_hex_uint64((uint32_t)(di_node));
  module_terminal_global_print_c_string(" to list.\n");

  // add copy to list
  if(module_pci_devices == NULL)
  {
    module_pci_devices = di_node;
  }
  else
  {
    // iterate to end of list
    module_pci_device_info * di_iterator = module_pci_devices;
    while(di_iterator->next_device != NULL)
    {
      di_iterator = di_iterator->next_device;
    }
    di_iterator->next_device = di_node;
  }
}
// -------------------------------------------------------------------------- //
void module_pci_detect_devices()
{
  const uint16_t invalid_vendor_id = 0xffff;
  module_pci_device_info di;
  module_pci_device_info_init(&di);
  for (uint8_t bus = 0; ; bus++) // if 255 break at end
  {
    for (uint8_t slot = 0; slot < 32; slot++)
    {
      uint32_t tmp_o0 = module_pci_config_read(bus, slot, 0, 0);
      di.vendor_id = tmp_o0 & 0xffff;
      if (di.vendor_id == invalid_vendor_id)
      {
        continue;
      }
      di.bus = bus;
      di.slot = slot;

      // see https://wiki.osdev.org/PCI#Header_Type_0x00
      uint32_t tmp_oc = module_pci_config_read(bus, slot, 0, 0xc);
      di.cache_line_size = tmp_oc & 0xff;
      di.latency_timer = (tmp_oc >> 8) & 0xff;
      di.header_type = (tmp_oc >> 16) & 0xff;
      di.bist = (tmp_oc >> 24) & 0xff;

      // is multi-function(8) or single-function(1) - check header bit
      di.is_multifunction_device = (di.header_type & 0x80) > 0;
      uint8_t no_functions = di.is_multifunction_device ? 8 : 1;

      for (uint8_t function = 0; function < no_functions; function++)
      {
        if ((module_pci_config_read(bus, slot, function, 0) & 0xffff)
          != 0xffff)
        {
          di.function = function;
          di.device_id = (tmp_o0 >> 16) & 0xffff;

          uint32_t tmp_o4 = module_pci_config_read(bus, slot, 0, 4);
          di.command = tmp_o4 & 0xffff;
          di.status = (tmp_o4 >> 16) & 0xffff;
          uint32_t tmp_o8 = module_pci_config_read(bus, slot, 0, 8);
          di.class_code = (tmp_o8 >> 24) & 0xff;
          di.subclass_code = (tmp_o8 >> 16) & 0xff;
          di.prog_if = (tmp_o8 >> 8) & 0xff;
          di.revision_id = tmp_o8 & 0xff;

          di.bar_0 = module_pci_config_read(bus, slot, 0, 0x10);

          module_pci_add_device_copy_to_list(&di);
        }
      }
    }

    // prevent overflow while iterating
    if(bus == 255)
    {
      break;
    }
  }
}
// -------------------------------------------------------------------------- //
void module_pci_free_devices()
{
  // iterate to end of list
  const module_pci_device_info * i = module_pci_devices;
  module_pci_devices = NULL;
  const module_pci_device_info * next = NULL;
  while(i != NULL)
  {
    next = i->next_device;
    module_terminal_global_print_c_string("Freeing PCI device ptr=");
    module_terminal_global_print_hex_uint64((uint32_t)(i));
    module_terminal_global_print_c_string(" from list.\n");
    free(i);
    // Move to next device
    i = next;
  }
}
// -------------------------------------------------------------------------- //
void module_pci_test()
{
  if(module_pci_devices == NULL)
  {
    module_terminal_global_print_c_string("PCI Device List is EMPTY ?!");
    return;
  }
  // if here, then not empty ...

  // iterate to end of list
  const module_pci_device_info * i = module_pci_devices;
  do
  {
    module_pci_print_device_info(i);
    // Move to next device
    i = i->next_device;
  }
  while(i->next_device != NULL);
  // Print last device, that has next = NULL
  module_pci_print_device_info(i);
}
// -------------------------------------------------------------------------- //

