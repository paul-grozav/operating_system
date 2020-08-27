// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_network.h"
#include "module_kernel.h"
#include "module_terminal.h"
#include "module_pci.h"
#include "module_heap.h"
// -------------------------------------------------------------------------- //
void enable_bus_mastering(uint64_t addr)
{
}
// -------------------------------------------------------------------------- //
void module_network_test()
{
  module_terminal_global_print_c_string("===- Network test -===\n");

  // address of network card
  uint8_t bus = 0;
  uint8_t slot = 3;
  uint8_t function = 0;

  // step 1 - enable PCI Bus Mastering
  {
    // First, you need to enable PCI Bus Mastering for this device. This allows the NIC to perform DMA. To do it, you can read the Command Register from the device's PCI Configuration Space, set bit 2 (bus mastering bit) and write the modified Command Register. Note this Command Register is that of the PCI Configuration Space and has nothing to do with the Command Register that will be evoked in the following sections: the latter is specific to the RTL8139, whereas every PCI device (not only NICs) have a PCI Configuration Space with a Command Register.
    // Some BIOS may enable Bus Mastering at startup, but some versions of qemu don't. You should thus be careful about this step.
    uint32_t state = module_pci_config_read(bus, slot, function, 4);
    state |= 0x04;
    module_pci_config_write(bus, slot, function, 4, state);
  }

  // step 2 - get iobase
  uint16_t iobase = 99;
  {
    // BAR0 from PCI is a 32 bit address, we can't fit it into a 16 bit address
    // We only use the first 16 bits, the last 16 bits are all 0.
    iobase = module_pci_config_read(bus, slot, function, 0x10) & ~1;
//    iobase = (module_pci_config_read(bus, slot, function, 0x10) & ~1) >> 16;
  }
  module_terminal_global_print_c_string("NET IO base=");
  module_terminal_global_print_uint64(iobase);
  module_terminal_global_print_c_string(" = ");
  module_terminal_global_print_hex_uint64(iobase);
  module_terminal_global_print_c_string(" = ");
  module_terminal_global_print_binary_uint64(iobase);
  module_terminal_global_print_c_string("\n");

  // step 3 - power on
  module_terminal_global_print_c_string("Powering on the Network device ...\n");
  {
    // Send 0x00 to the CONFIG_1 register (0x52) to set the LWAKE + LWPTN to active high. this should essentially *power on* the device.
    module_kernel_out_8(iobase + 0x52, 0);
  }
  module_terminal_global_print_c_string("Network device powered on.\n");

  // step 4 - Software Reset !
  module_terminal_global_print_c_string("Resetting network device ...\n");
  {
    //Next, we should do a software reset to clear the RX and TX buffers and set everything back to defaults. Do this to eliminate the possibility of there still being garbage left in the buffers or registers on power on.
    // Sending 0x10 to the Command register (0x37) will send the RTL8139 into a software reset. Once that byte is sent, the RST bit must be checked to make sure that the chip has finished the reset. If the RST bit is high (1), then the reset is still in operation.
    // NB: There is a minor bug in Qemu. If you check the command register before performing a soft reset, you may find the RST bit is high (1). Just ignore it and carry on with the initialization procedure.
    if( (module_kernel_in_8(iobase + 0x37) & 0x10) == 0 )
    {
      module_kernel_out_8(iobase + 0x37, 0x10); // reset
      while (module_kernel_in_8(iobase + 0x37) & 0x10)
      {
        // await reset
      }
    }
  }
  module_terminal_global_print_c_string("Network device is now (re)set.\n");

  // step 5 - Init Receive buffer
  module_terminal_global_print_c_string("Initializing network RX buffer ...\n");
  module_heap_heap_bm nic_heap;
  char *rx_buffer;
  {
    module_heap_init(&nic_heap);
    module_heap_add_block(&nic_heap, 10*1024*1024, 1024*1024, 16);
    rx_buffer = (char*)module_heap_alloc(&nic_heap, 8*1024 + 16);
    // For this part, we will send the chip a memory location to use as its receive buffer start location. One way to do it, would be to define a buffer variable and send that variables memory location to the RBSTART register (0x30).
    // Note that 'rx_buffer' needs to be a pointer to a "physical address". In this case a size of 8192 + 16 (8K + 16 bytes) is recommended, see below.
     module_kernel_out_32(iobase + 0x30, (uint32_t)(rx_buffer)); // send uint32_t memory location to RBSTART (0x30)
  }
  module_terminal_global_print_c_string("NIC RX buff initialized.\n");

  // step 6 - enable RX & TX
  module_terminal_global_print_c_string("NIC enable RX & TX ...\n");
  {
    module_kernel_out_16(iobase + 0x3c, 0x0005); // configure interrupts txok and rxok
    module_kernel_out_32(iobase + 0x40, 0x600); // send larger DMA bursts
    module_kernel_out_32(iobase + 0x44, 0x68f); // accept all packets + unlimited DMA
    module_kernel_out_8(iobase + 0x37, 0x0c); // enable rx and tx
  }
  module_terminal_global_print_c_string("NIC enabled RX & TX.\n");

  // step 7 - get MAC address
  // you can run setting a given MAC using:
  // ~>docker exec -it os bash /mnt/run.sh && qemu-system-i386 -cdrom ~/data/h313/network/docker/research/os/build_machine/fs/project/build/bootable.iso -boot d -netdev user,id=mynet0 -device rtl8139,netdev=mynet0,mac=00:01:02:13:14:fa
  module_terminal_global_print_c_string("MAC address = ");
  {
    for (uint8_t i=0; i<6; i++)
    {
      uint8_t mac_byte = module_kernel_in_8(iobase + i);
      module_terminal_global_print_hex_uint64(mac_byte);
      if(i<5)
      {
        module_terminal_global_print_c_string(":");
      }
    }
  }
  module_terminal_global_print_c_string(" .\n");

  // free NIC buffer memory
  module_terminal_global_print_c_string("NIC RX buff free ...\n");
  module_heap_free(&nic_heap, rx_buffer);
  module_terminal_global_print_c_string("NIC RX buff freed.\n");
}
// -------------------------------------------------------------------------- //

