// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_kernel.h"
#include "module_base.h"
#include "module_terminal.h"
#include "module__pci.h"
#include "module_heap.h"
#include "module_interrupt.h"
#include "module__network.h"
#include "module__driver__rtl8139.h"
// -------------------------------------------------------------------------- //
module__driver__rtl8139__instance * module__driver__rtl8139__instances = NULL;
// -------------------------------------------------------------------------- //
#define MASTER_DATA 0x21
#define SLAVE_DATA 0xA1
void pic_irq_unmask(int irq)
{
  unsigned char mask;

  if (irq > 15 || irq < 0)
  {
//    panic("pic: can't unmask irq %i\n", irq);
    return;
  }

  if (irq >= 8)
  {
    mask = module_kernel_in_8(SLAVE_DATA);
    mask &= ~(1 << (irq - 8));
    module_kernel_out_8(SLAVE_DATA, mask);
  }
  else
  {
    mask = module_kernel_in_8(MASTER_DATA);
    mask &= ~(1 << (irq));
    module_kernel_out_8(MASTER_DATA, mask);
  }
}
// -------------------------------------------------------------------------- //
static inline uintptr_t round_down(uintptr_t val, uintptr_t place)
{
  return val & ~(place - 1);
}
// -------------------------------------------------------------------------- //
static inline uintptr_t round_up(uintptr_t val, uintptr_t place)
{
  return round_down(val + place - 1, place);
}
// -------------------------------------------------------------------------- //
#define ETH_MTU 1536
void module__driver__rtl8139__send_packet(
  const module__network__data__packet * const p,
  module__driver__rtl8139__instance * driver)
{
  if(!(p->length > 0 && p->length < ETH_MTU))
  {
    // problem
    module_terminal_global_print_c_string("Problem sending packet with length=");
    module_terminal_global_print_uint64(p->length);
    module_terminal_global_print_c_string("\n");
  }
//  virt_addr_t data = (virt_addr_t)p->buffer;
//  phys_addr_t phy_data = vmm_virt_to_phy(data);

//  module_terminal_global_print_c_string("SND pk len=");
//  module_terminal_global_print_uint64(p->length);
//  module_terminal_global_print_c_string("\n");
//  module_terminal_global_print_c_string("tx_slot=");
//  module_terminal_global_print_uint64(tx_slot);
//  module_terminal_global_print_c_string("\n");
  uint16_t tx_addr_off = 0x20 + (driver->tx_slot - 1) * 4;
//  module_terminal_global_print_c_string("tx_addr_off=");
//  module_terminal_global_print_hex_uint64(tx_addr_off);
//  module_terminal_global_print_c_string("\n");
  uint16_t ctrl_reg_off = 0x10 + (driver->tx_slot - 1) * 4;
//  module_terminal_global_print_c_string("ctrl_reg_off=");
//  module_terminal_global_print_hex_uint64(ctrl_reg_off);
//  module_terminal_global_print_c_string("\n");

  module_kernel_out_32(driver->iobase + tx_addr_off, (uint32_t)(p->buffer));
  module_kernel_out_32(driver->iobase + ctrl_reg_off, p->length);

  // TODO: could let this happen async and just make sure the descriptor
  // is done when we loop back around to it.

  size_t take = 0; // OWNership of data
  take = 0b10000000000000;
//  take = 0x100; // 0b100000000

  size_t conf = 0;
  conf = 0b1000000000000000;
//  conf = 0x400;
  uint32_t x = 0;
  x = module_kernel_in_32(driver->iobase + ctrl_reg_off);
  while ( !(x & take) )
  {
    // await device taking packet
    x = module_kernel_in_32(driver->iobase + ctrl_reg_off);
  }
//  module_terminal_global_print_c_string("x=");
//  module_terminal_global_print_binary_uint64(x);
//  module_terminal_global_print_c_string("\n");
  x = module_kernel_in_32(driver->iobase + ctrl_reg_off);
  while ( !(x & conf) )
  {
    // await send confirmation
    x = module_kernel_in_32(driver->iobase + ctrl_reg_off);
  }
  // gets out with 000 ... 1010 0000 0011 1000
//  module_terminal_global_print_c_string("x=");
//  module_terminal_global_print_binary_uint64(x);
//  module_terminal_global_print_c_string("\n");

  // rtl8139 has 4 sending buffers see:
  // https://wiki.osdev.org/RTL8139#Transmitting_Packets
  // slots are 1, 2, 3, 4 - MUST be used in sequence
  driver->tx_slot %= 4;
  driver->tx_slot += 1;
}
// -------------------------------------------------------------------------- //
#define RX_OK 0x01
#define RX_ERR 0x02
#define TX_OK 0x04
#define RCR_AAP (1 << 0) // Accept All Packets
#define RCR_APM (1 << 1) // Accept Physical Match Packets
#define RCR_AM (1 << 2) // Accept Multicast Packets
#define RCR_AB (1 << 3) // Accept Broadcast Packets
#define RCR_WRAP (1 << 7) // Wrap packets too long
#define TX_ERR 0x08
#define ETH_MIN_LENGTH 60
#define ETH_FRAME_LEGTH 1514
#define RX_STATUS_OK 0x1
#define RX_BAD_ALIGN 0x2
#define RX_CRC_ERR 0x4
#define RX_TOO_LONG 0x8
#define RX_RUNT 0x10
#define RX_BAD_SYMBOL 0x20
#define RX_BROADCAST 0x2000
#define RX_PHYSICAL 0x4000
#define RX_MULTICAST 0x8000
#define TX_STATUS 0x10
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__generic_interrupt_handler(
  module_interrupt_registers_t x)
{
  (void)x; // avoid unused param

  // iterate to last in list
  module__driver__rtl8139__instance * i = module__driver__rtl8139__instances;
  do
  {
    // check if interrupt is for this instance / driver
    uint16_t interrupt_flags = module_kernel_in_16(i->iobase + 0x3e);
//    module_terminal_global_print_c_string("NIC IRQ flag=");
//    module_terminal_global_print_binary_uint64(interrupt_flag);
//    module_terminal_global_print_c_string("\n");
    if (interrupt_flags != 0)
    {
      module__driver__rtl8139__interrupt_handler(i, interrupt_flags);
      return;
    }
//    else
//    {
//      module_terminal_global_print_c_string("This card did not interrupt,"
//        " nothing to do.\n");
//    }
    i= i->next_driver;
  }
  while(i != NULL);
  module_terminal_global_print_c_string("ERROR: Couldn't find the RTL8139 card"
    " for which we had an interrupt.\n");
}
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__interrupt_handler(
  module__driver__rtl8139__instance * driver, const uint16_t flags)
{
  if (flags & RX_OK)
  {
    module_terminal_global_print_c_string("Packet received.\n");
  }
  if (flags & RX_ERR)
  {
    module_terminal_global_print_c_string("Packet receive error.\n");
  }
  if (flags & TX_ERR)
  {
    module_terminal_global_print_c_string("Packet sending error.\n");
  }
  if (flags & TX_OK)
  {
    module_terminal_global_print_c_string("Packet sent.\n");
    module_kernel_in_32(driver->iobase + TX_STATUS + (driver->tx_slot-1) * 4);
  }

  // Acknowledge the interrupt
  module_kernel_out_16(driver->iobase + 0x3e, flags);

  if (!(flags & 1))
  {
    module_terminal_global_print_c_string("This card interrupted, but there is"
      " no incoming packet. Ack and out.\n");
    // Acknowledge the interrupt
//    module_kernel_out_16(iobase + 0x3e, interrupt_flag);
    return;
  }

  while((module_kernel_in_8(driver->iobase + 0x37) & 1) == 0)
  { // while RX NOT empty
    module_terminal_global_print_c_string("This card interrupted, and there is"
      " a packet. Processing it ...\n");
    {
//      module_terminal_global_print_c_string("rx_index=");
//      module_terminal_global_print_uint64(rx_index);
//      module_terminal_global_print_c_string("\n");
      const uint16_t * const packet_header = (const uint16_t * const)(
        driver->rx_buffer + driver->rx_index);
      const uint16_t flags = packet_header[0];
      const uint16_t length = packet_header[1];
//      module_terminal_global_print_c_string("PP.flags=");
//      module_terminal_global_print_uint64(flags);
//      module_terminal_global_print_c_string("\n");
      module_terminal_global_print_c_string("PP.length=");
      module_terminal_global_print_uint64(length);
      module_terminal_global_print_c_string("\n");

      if ((flags & (RX_BAD_SYMBOL | RX_RUNT | RX_TOO_LONG |
        RX_CRC_ERR | RX_BAD_ALIGN)) ||
        (length < ETH_MIN_LENGTH) ||
        (length > ETH_FRAME_LEGTH))
      {
        module_terminal_global_print_c_string("RTL8139 packet error.\n");
        // Acknowledge the interrupt
//        module_kernel_out_16(iobase + 0x3e, interrupt_flag);
        return;
      }

      module__network__data__packet * pk = NULL;
//      uint32_t pk_buffer = (uint32_t)(rx_buffer) + rx_index + 4;
      if ((flags & 1) == 0)
      {
        module_terminal_global_print_c_string("Got a bad packet.\n");
      }
      else
      {
        pk = (module__network__data__packet*)malloc(
          sizeof(module__network__data__packet) + length);
        if(pk == NULL)
        {
          module_terminal_global_print_c_string("NIC packet alloc is NULL!\n");
        }
  //      pk->from = dev;
        pk->length = length - 4;
//        module_terminal_global_print_c_string("---PACKET_BEGIN---\n");
//        print_hex_bytes2(rx_buffer + rx_index + 4, length);
//        module_terminal_global_print_c_string("---PACKET_END---\n");
        // +4 to skip packet header read above ( 2*16bit ints = 4 bytes )
        module_kernel_memcpy(driver->rx_buffer + driver->rx_index + 4,
          pk->buffer, pk->length);
      }
      module__network__process_ethernet_packet(pk, driver->ethernet_interface);
      // end
      free(pk);
      driver->rx_index += round_up(length + 4, 4);
      driver->rx_index %= 8192;
      module_kernel_out_16(driver->iobase + 0x38, driver->rx_index - 16);
    }
  }

  // Acknowledge the interrupt
//  module_kernel_out_16(iobase + 0x3e, interrupt_flag);
//  module_interrupt_enable(); // needed?
}
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__global_list_add_driver(
  module__driver__rtl8139__instance * driver)
{
  if(module__driver__rtl8139__instances == NULL)
  {
    module__driver__rtl8139__instances = driver;
    return;
  }

  // iterate to last in list
  module__driver__rtl8139__instance * i = module__driver__rtl8139__instances;
  do
  {
    i = i->next_driver;
  }
  while(i->next_driver != NULL);
  // append after last
  i->next_driver = driver;
}
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__driver_init(const uint8_t bus, const uint8_t slot,
  const uint8_t function,
  module__network__data__mac_address * const mac_address, void ** driver_ptr,
  void * ethernet_interface)
{
  *driver_ptr = malloc(sizeof(module__driver__rtl8139__instance));
  module__driver__rtl8139__instance * driver =
    (module__driver__rtl8139__instance *)(*driver_ptr);
  // set struct default values
  driver->iobase = 99;
  driver->rx_buff_size = 8192 + 16 + 1500; // 8*1024 + 16
  driver->rx_buffer = NULL;
  driver->rx_index = 0;
  driver->tx_slot = 0;
  driver->ethernet_interface = ethernet_interface;
  driver->next_driver = NULL;
  module_terminal_global_print_c_string("Driver instance = ");
  module_terminal_global_print_hex_uint64((uint64_t)(driver));
  module_terminal_global_print_c_string("\n");
  module__driver__rtl8139__global_list_add_driver(driver);
  // PCI address of network card: bus, slot, function

  // step 1 - enable PCI Bus Mastering
  {
    // First, you need to enable PCI Bus Mastering for this device. This allows the NIC to perform DMA. To do it, you can read the Command Register from the device's PCI Configuration Space, set bit 2 (bus mastering bit) and write the modified Command Register. Note this Command Register is that of the PCI Configuration Space and has nothing to do with the Command Register that will be evoked in the following sections: the latter is specific to the RTL8139, whereas every PCI device (not only NICs) have a PCI Configuration Space with a Command Register.
    // Some BIOS may enable Bus Mastering at startup, but some versions of qemu don't. You should thus be careful about this step.
    uint32_t state = module__pci__config_read(bus, slot, function, 4);
    state |= 0x04;
    module__pci__config_write(bus, slot, function, 4, state);
  }

  // step 2 - get iobase
  {
    driver->iobase = //(uint16_t)(
      module__pci__config_read(bus, slot, function, 0x10) & //(uint32_t)(
        ~1
//     ))
    ;
  }
  module_terminal_global_print_c_string("NET IO base=");
  module_terminal_global_print_uint64(driver->iobase);
  module_terminal_global_print_c_string(" = ");
  module_terminal_global_print_hex_uint64(driver->iobase);
  module_terminal_global_print_c_string(" = ");
  module_terminal_global_print_binary_uint64(driver->iobase);
  module_terminal_global_print_c_string("\n");

  // step 4 - get MAC address
  // you can run setting a given MAC using:
  // ~>docker exec -it os bash /mnt/run.sh && qemu-system-i386 -cdrom ~/data/h313/network/docker/research/os/build_machine/fs/project/build/bootable.iso -boot d -netdev user,id=mynet0 -device rtl8139,netdev=mynet0,mac=00:01:02:13:14:fa
  module_terminal_global_print_c_string("MAC address = ");
//  module__network__data__mac_address ma;
  {
    for (uint8_t i=0; i<6; i++)
    {
      uint8_t mac_byte = module_kernel_in_8(driver->iobase + i);
      mac_address->data[i] = mac_byte;
    }
    module__network__print_mac(mac_address);
  }
  module_terminal_global_print_c_string(" .\n");

  // step 5 - power on
  module_terminal_global_print_c_string("Powering on the Network device ...\n");
  {
    // Send 0x00 to the CONFIG_1 register (0x52) to set the LWAKE + LWPTN to active high. this should essentially *power on* the device.
    module_kernel_out_8(driver->iobase + 0x52, 0);
  }
  module_terminal_global_print_c_string("Network device powered on.\n");

  // step 6 - Software Reset !
  module_terminal_global_print_c_string("Resetting network device ...\n");
  {
    //Next, we should do a software reset to clear the RX and TX buffers and set everything back to defaults. Do this to eliminate the possibility of there still being garbage left in the buffers or registers on power on.
    // Sending 0x10 to the Command register (0x37) will send the RTL8139 into a software reset. Once that byte is sent, the RST bit must be checked to make sure that the chip has finished the reset. If the RST bit is high (1), then the reset is still in operation.
    // NB: There is a minor bug in Qemu. If you check the command register before performing a soft reset, you may find the RST bit is high (1). Just ignore it and carry on with the initialization procedure.
    if( (module_kernel_in_8(driver->iobase + 0x37) & 0x10) == 0 )
    {
      module_kernel_out_8(driver->iobase + 0x37, 0x10); // reset
      while (module_kernel_in_8(driver->iobase + 0x37) & 0x10)
      {
        // await reset
      }
    }
  }
  module_terminal_global_print_c_string("Network device is now (re)set.\n");

  // step 7 - Init Receive buffer
  module_terminal_global_print_c_string("Initializing network RX buffer ...\n");
  {
//    module_heap_init(&nic_heap);
//    module_heap_add_block(&nic_heap, 0x110000, 1024*1024, 16);
    driver->rx_buffer = (uint8_t*)malloc(driver->rx_buff_size);
    if(driver->rx_buffer == 0)
    {
      module_terminal_global_print_c_string("NIC rx_buffer= NULL !!!");
      // free stuff and return error TODO !!!
    }
    module_kernel_memset(driver->rx_buffer, 0, driver->rx_buff_size);
//    print_hex_bytes(rx_buffer, 64);//rx_buff_size);
    // For this part, we will send the chip a memory location to use as its receive buffer start location. One way to do it, would be to define a buffer variable and send that variables memory location to the RBSTART register (0x30).
    // Note that 'rx_buffer' needs to be a pointer to a "physical address". In this case a size of 8192 + 16 (8K + 16 bytes) is recommended, see below.
    module_kernel_out_32(driver->iobase + 0x30, (uint32_t)(driver->rx_buffer)); // send uint32_t memory location to RBSTART (0x30)
    module_kernel_out_32(driver->iobase + 0x38, 0); // buffer pointer
    module_kernel_out_32(driver->iobase + 0x3a, 0); // buffer address
  }
  module_terminal_global_print_c_string("NIC rx_buffer=");
  module_terminal_global_print_uint64((uint64_t)((uint32_t)((uint8_t*)(driver->rx_buffer))));
  module_terminal_global_print_c_string("\n");
  module_terminal_global_print_c_string("NIC RX buff initialized.\n");

  // step 8 - enable RX & TX
  module_terminal_global_print_c_string("NIC enable RX & TX ...\n");
  {
    // 0x3c = Interrupt Mask Register
    module_kernel_out_16(driver->iobase + 0x3c, (RX_OK | TX_OK | TX_ERR));
    module_kernel_out_32(driver->iobase + 0x40, 0x600); // send larger DMA bursts
    // 0x44 = Receive Config Register
    module_kernel_out_32(driver->iobase + 0x44, 0x68f);
//      (RCR_AAP | RCR_APM | RCR_AM | RCR_AB | RCR_WRAP)); // accept all packets + unlimited DMA
//    module_kernel_out_32(iobase + 0x4c, 0x0); // RX_MISSED
    module_kernel_out_8(driver->iobase + 0x37, 0x0c); // enable rx and tx bits high
  }
  driver->tx_slot = 1;
  module_terminal_global_print_c_string("NIC enabled RX & TX.\n");


  // step 3 - set IRQ handler
  {
    uint8_t irq = module__pci__config_read(bus, slot, function, 0x3c) &0xff;
    module_terminal_global_print_c_string("Read IRQ=");
    module_terminal_global_print_uint64(irq);
    module_terminal_global_print_c_string("\n");
    pic_irq_unmask(irq);

    // See : https://forum.osdev.org/viewtopic.php?f=1&t=33260
    // and:
    // https://github.com/narke/Aragveli/blob/master/src/extra/drivers/rtl8139.c
    irq += 32; // 43 is actually triggered instead of 11
    module_interrupt_register_interrupt_handler(irq,
      module__driver__rtl8139__generic_interrupt_handler);
    module_interrupt_enable_irq(irq);
  }

  // step 9 - send data over NIC
//  module_terminal_global_print_c_string("NIC sending data ...\n");
//  send_data(ma);
//  module_terminal_global_print_c_string("NIC data sent.\n");

  // free NIC buffer memory
//  module_terminal_global_print_c_string("NIC RX buff free ...\n");
//  module_heap_free(&nic_heap, rx_buffer);
//  module_terminal_global_print_c_string("NIC RX buff freed.\n");
}
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__driver_free(
  module__driver__rtl8139__instance * driver)
{
  module_terminal_global_print_c_string("Freeing RTL8139 dirver=");
  module_terminal_global_print_hex_uint64((uint32_t)(driver));
  module_terminal_global_print_c_string(" done.\n");
  free(driver->rx_buffer);
  module_kernel_memset(driver->rx_buffer, 0, driver->rx_buff_size);

  free(driver);
  module_kernel_memset(driver, 0, sizeof(module__driver__rtl8139__instance));
}
// -------------------------------------------------------------------------- //
void module__driver__rtl8139__driver_free_all()
{
  // iterate to last in list
  module__driver__rtl8139__instance * i = module__driver__rtl8139__instances;
  module__driver__rtl8139__instance * next = NULL;
  do
  {
    next = i->next_driver;
    module__driver__rtl8139__driver_free(i);
    i = next;
  }
  while(i != NULL);
}
// -------------------------------------------------------------------------- //

