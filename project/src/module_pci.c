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
#include "module_terminal.h"
#include "module_base.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//#define outportb(a,b) module_kernel_out_8(a,b)
//#define inportb(a,b) module_kernel_in_8(a,b)

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

          module_terminal_global_print_c_string("Detected PCI device:");
          module_terminal_global_print_c_string("   bus=");
          module_terminal_global_print_uint8(di.bus);
          module_terminal_global_print_c_string("   slot=");
          module_terminal_global_print_uint64(di.slot);
          module_terminal_global_print_c_string("   vendor=");
          module_terminal_global_print_hex_uint64(di.vendor_id);
          module_terminal_global_print_c_string("   device=");
          module_terminal_global_print_hex_uint64(di.device_id);
          module_terminal_global_print_c_string("   class=");
          module_terminal_global_print_hex_uint64(di.class_code);
          module_terminal_global_print_c_string("   subclass=");
          module_terminal_global_print_hex_uint64(di.subclass_code);
          module_terminal_global_print_c_string("   is_multi_function=");
          module_terminal_global_print_uint8(di.is_multifunction_device);
          module_terminal_global_print_c_string("   function=");
          module_terminal_global_print_uint8(di.function);
          module_terminal_global_print_c_string("   command=");
          module_terminal_global_print_binary_uint64(di.command);
          module_terminal_global_print_c_string("   status=");
          module_terminal_global_print_uint64(di.status);
          module_terminal_global_print_c_string("   prog_if=");
          module_terminal_global_print_uint8(di.prog_if);
          module_terminal_global_print_c_string("   revision_id=");
          module_terminal_global_print_uint8(di.revision_id);
          module_terminal_global_print_c_string("   latency_timer=");
          module_terminal_global_print_uint8(di.latency_timer);
          module_terminal_global_print_c_string("   cache_line_size=");
          module_terminal_global_print_uint8(di.cache_line_size);
          module_terminal_global_print_c_string("   header_type=");
          module_terminal_global_print_binary_uint64(di.header_type);
          module_terminal_global_print_c_string("   BIST=");
          module_terminal_global_print_binary_uint64(di.bist);
          module_terminal_global_print_c_string("   BAR0=");
          module_terminal_global_print_hex_uint64(di.bar_0);
          module_terminal_global_print_c_string(" .\n\n");
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
void module_pci_test()
{
  module_pci_detect_devices();
}
// -------------------------------------------------------------------------- //

