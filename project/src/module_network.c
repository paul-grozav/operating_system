// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// See : https://opensource.apple.com/source/boot/boot-83.2/i386/libsaio
// /legacy/PCI.h.auto.html
#include "module_network.h"
#include "module_kernel.h"
#include "module_terminal.h"
#include "module_base.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
#define outportb(a,b) module_kernel_out_8(a,b)
#define inportb(a,b) module_kernel_in_8(a,b)

#define OFFSET_DEVICEID 0x02
#define OFFSET_VENDORID 0x00
#define MULTI_FUNCTION 0x80
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
#define PCI_CONFIG 0xCF8
#define PCI_DATA 0xCFC
// config Offsets
#define PCI_VENDOR_DEVICE 0x00
#define PCI_CLASS_SUBCLASS 0x08
#define PCI_BAR0 0x10
#define PCI_BAR1 PCI_BAR0 + 4
#define PCI_BAR2 PCI_BAR1 + 4
#define PCI_BAR3 PCI_BAR2 + 4
#define PCI_BAR4 PCI_BAR3 + 4
#define PCI_BAR5 PCI_BAR4 + 4
// -------------------------------------------------------------------------- //
uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
  uint32_t reg = 0x80000000;
  reg |= (bus & 0xFF) << 16;
  reg |= (device & 0x1F) << 11;
  reg |= (function & 0x7) << 8;
  reg |= (offset & 0xFF) & 0xFC;
  module_kernel_out_32(PCI_CONFIG, reg);
  return module_kernel_in_32(PCI_DATA);
}
// -------------------------------------------------------------------------- //
//void pci_write(uint32_t bus, uint32_t device, uint32_t function,
//  uint32_t offset, uint32_t data)
//{
//  uint32_t reg = 0x80000000;
//  reg |= (bus & 0xFF) << 16;
//  reg |= (device & 0x1F) << 11;
//  reg |= (function & 0x7) << 8;
//  reg |= offset & 0xFC;
//  module_kernel_out_32(PCI_CONFIG, reg);
//  module_kernel_out_32(PCI_DATA, data);
//}
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
void detect_pci_devices()
{
  const uint16_t invalid_vendor_id = 0xffff;
  for (uint8_t bus = 0; bus <=255; bus++)
  {
    for (uint8_t device = 0; device < 32; device++)
    {
      uint32_t tmp_o0 = pci_read(bus, device, 0, 0);
      uint16_t vendor_id = tmp_o0 & 0xffff;
      if (vendor_id == invalid_vendor_id)
      {
        continue;
      }

      // see https://wiki.osdev.org/PCI#Header_Type_0x00
      uint32_t header_type = 0xff & (pci_read(bus, device, 0, 0xc) >> 16);

      // is multi-function(8) or single-function(1)
      uint8_t functions = (header_type & 0x80) ? 8 : 1;

      for (uint8_t function = 0; function < functions; function++)
      {
        if ((pci_read(bus, device, function, 0) & 0xffff) != 0xffff)
        {
//          uint32_t vendorID = tmp_o0 & 0xffff;
          uint16_t device_id = (tmp_o0 >> 16) & 0xffff;

          uint32_t tmp_o8 = pci_read(bus, device, 0, 8);
          uint8_t class_code = (tmp_o8 >> 24) & 0xff;
          uint8_t subclass_code = (tmp_o8 >> 16) & 0xff;

          module_terminal_global_print_c_string("Detected device: vendor ");
          module_terminal_global_print_hex_uint64(vendor_id);
          module_terminal_global_print_c_string("   device ");
          module_terminal_global_print_hex_uint64(device_id);
          module_terminal_global_print_c_string("   class ");
          module_terminal_global_print_hex_uint64(class_code);
          module_terminal_global_print_c_string("   subclass ");
          module_terminal_global_print_hex_uint64(subclass_code);
          module_terminal_global_print_c_string(".\n");
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
void module_network_test()
{
  detect_pci_devices();
}
// -------------------------------------------------------------------------- //

