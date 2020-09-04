// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_network.h"
#include "module_kernel.h"
#include "module_base.h"
#include "module_terminal.h"
#include "module_pci.h"
#include "module_heap.h"
#include "module_interrupt.h"
// -------------------------------------------------------------------------- //
uint16_t iobase = 99;
module_heap_heap_bm nic_heap;
uint8_t *rx_buffer = NULL;
size_t rx_index = 0;
size_t rx_buff_size = 8192 + 16 + 1500; // 8*1024 + 16
// -------------------------------------------------------------------------- //
// -----
typedef uint32_t be32;
typedef uint16_t be16;

#define __const_htons(x) ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8));
#define __const_ntohs __const_htons

static inline uint16_t ntohs(uint16_t x) {
        return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}

static inline uint16_t htons(uint16_t x) {
        return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}

static inline uint32_t ntohl(uint32_t x) {
        return (
                ((x & 0xFF000000) >> 24) |
                ((x & 0x00FF0000) >> 8)  |
                ((x & 0x0000FF00) << 8)  |
                ((x & 0x000000FF) << 24)
        );
}

static inline uint32_t htonl(uint32_t x) {
        return (
                ((x & 0xFF000000) >> 24) |
                ((x & 0x00FF0000) >> 8)  |
                ((x & 0x0000FF00) << 8)  |
                ((x & 0x000000FF) << 24)
        );
}

#define MASTER_DATA 0x21
#define SLAVE_DATA 0xA1
void pic_irq_unmask(int irq) {
        unsigned char mask;

        if (irq > 15 || irq < 0)
        {
//                panic("pic: can't unmask irq %i\n", irq);
          return;
        }

        if (irq >= 8) {
                mask = module_kernel_in_8(SLAVE_DATA);
                mask &= ~(1 << (irq - 8));
                module_kernel_out_8(SLAVE_DATA, mask);
        } else {
                mask = module_kernel_in_8(MASTER_DATA);
                mask &= ~(1 << (irq));
                module_kernel_out_8(MASTER_DATA, mask);
        }
}

enum ethertype {
    ETH_IP = 0x0800,
    ETH_ARP = 0x0806,
};

struct __attribute__((__packed__)) mac_address
{
  char data[6];
};
static const struct mac_address broadcast_mac =
  {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
static const struct mac_address zero_mac = {{0, 0, 0, 0, 0, 0}};
void print_mac(const struct mac_address * const ma)
{
  char buffer[20+1];
  size_t l = 0;
  for (uint8_t i=0; i<6; i++)
  {
    l = module_base_uint64_to_ascii_base16((uint8_t)(ma->data[i]), buffer);
    if(l == 1)
    {
      buffer[2] = '\0';
      buffer[1] = buffer[0];
      buffer[0] = '0';
    } else {
      buffer[l] = '\0';
    }
    module_terminal_global_print_c_string(buffer);
    if(i<5)
    {
      module_terminal_global_print_c_string(":");
    }
  }
}
void print_ip(const uint32_t ip)
{
  for (uint8_t i=0; i<4; i++)
  {
    module_terminal_global_print_uint64( ((const uint8_t * const)(&ip)) [i] );
    if(i < 3)
    {
      module_terminal_global_print_c_string(".");
    }
  }
}
struct pkb
{
  struct net_device *from;
  void* queue;
  int refcount;
  uint8_t user_anno[32];
  int length; // -1 if unknown
  uint8_t buffer[];
};
struct __attribute__((__packed__)) ethernet_header
{
  struct mac_address destination_mac;
  struct mac_address source_mac;
  be16 ethertype;
  char data[];
};
enum arp_op {
    ARP_REQ = 1,
    ARP_RESP = 2,
};
struct __attribute__((__packed__)) arp_header {
    // eth_hdr
    be16 hw_type;
    be16 proto;
    uint8_t hw_size;
    uint8_t proto_size;
    be16 op;
    struct mac_address sender_mac;
    be32 sender_ip;
    struct mac_address target_mac;
    be32 target_ip;
};
// ----
void send_data(struct mac_address ma)
{
  uint8_t slot = 0;
  uint16_t tx_addr_off = 0x20 + (slot - 1) * 4;
  uint16_t ctrl_reg_off = 0x10 + (slot - 1) * 4;


  size_t pk_size = sizeof(struct ethernet_header)
    + sizeof(struct arp_header);
  char send_data[pk_size];
  struct ethernet_header *eh = (struct ethernet_header*)(send_data);
  eh->source_mac = ma;
  eh->destination_mac = broadcast_mac;
  eh->ethertype = htons(ETH_ARP);

  struct arp_header *r_arp = (struct arp_header*)(send_data
    + sizeof(struct ethernet_header));
  r_arp->hw_type = htons(1);      // eth_hdr
  r_arp->proto = htons(0x0800);   // ip_hdr
  r_arp->hw_size = 6;
  r_arp->proto_size = 4;
  r_arp->op = htons(ARP_REQ);
  r_arp->sender_mac = ma;
  r_arp->sender_ip = 0xc0a8c845;
  r_arp->target_mac = zero_mac;
  r_arp->target_ip = 0xc0a8c846;

  module_kernel_out_32(iobase + tx_addr_off, (uint32_t)(send_data));
  module_kernel_out_32(iobase + ctrl_reg_off, pk_size);

  // await device taking packet
  while (module_kernel_in_8(iobase + ctrl_reg_off) & 0x100);
  // await send confirmation
  while (module_kernel_in_8(iobase + ctrl_reg_off) & 0x400);
}
// -------------------------------------------------------------------------- //
void print_hex_bytes(const uint8_t * const base, const size_t count)
{
  char buffer[20+1];
  size_t l = 0;
  for (size_t i=0; i<count; i++)
  {
    l = module_base_uint64_to_ascii_base16(*(base + i), buffer);
    if(l == 1)
    {
      buffer[2] = '\0';
      buffer[1] = buffer[0];
      buffer[0] = '0';
    } else {
      buffer[l] = '\0';
    }
    module_terminal_global_print_c_string(buffer);
    if(i < count-1)
    {
      module_terminal_global_print_c_string(" ");
    }
  }
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
static inline uintptr_t round_down(uintptr_t val, uintptr_t place) {
  return val & ~(place - 1);
}
static inline uintptr_t round_up(uintptr_t val, uintptr_t place) {
  return round_down(val + place - 1, place);
}
// -------------------------------------------------------------------------- //
static uint8_t rx_empty() {
  return (module_kernel_in_8(iobase + 0x37) & 1) != 0;
}
// -------------------------------------------------------------------------- //
const struct ethernet_header * eth_hdr(const struct pkb * const pk)
{
  return (const struct ethernet_header * const)&(pk->buffer);
}
// -------------------------------------------------------------------------- //
void print_eth_hdr(const struct ethernet_header * const h)
{
  module_terminal_global_print_c_string("ethernet_header");
  module_terminal_global_print_c_string("{ \"source_mac\": \"");
  print_mac(&(h->source_mac));
  module_terminal_global_print_c_string("\", \"destination_mac\": \"");
  print_mac(&(h->destination_mac));
  module_terminal_global_print_c_string("\", \"type\": \"");
  const uint16_t eth_type = ntohs(h->ethertype);
  if(eth_type == ETH_ARP)
  {
    module_terminal_global_print_c_string("ARP");
  } else if(eth_type == ETH_IP) {
    module_terminal_global_print_c_string("IP");
  } else {
    module_terminal_global_print_c_string("UNKNOWN");
  }
  module_terminal_global_print_c_string("(");
  module_terminal_global_print_uint64(eth_type);
  module_terminal_global_print_c_string(")");
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void print_arp_header(const struct arp_header * const h)
{
  module_terminal_global_print_c_string("arp_header");
  module_terminal_global_print_c_string("{ \"sender_ip\": \"");
  print_ip(h->sender_ip);
  module_terminal_global_print_c_string("\", \"target_ip\": \"");
  print_ip(h->target_ip);
  module_terminal_global_print_c_string("\", \"nothing\": \"");
/*
  print_mac(&(h->destination_mac));
  module_terminal_global_print_c_string("\", \"type\": \"");
  const uint16_t eth_type = ntohs(h->ethertype);
  if(eth_type == ETH_ARP)
  {
    module_terminal_global_print_c_string("ARP");
  } else if(eth_type == ETH_IP) {
    module_terminal_global_print_c_string("IP");
  } else {
    module_terminal_global_print_c_string("UNKNOWN");
  }
  module_terminal_global_print_c_string("(");
  module_terminal_global_print_uint64(eth_type);
  module_terminal_global_print_c_string(")");
*/
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
const struct arp_header * arp_hdr(const struct pkb * const pk)
{
  return (const struct arp_header * const)(
    pk->buffer + sizeof(struct ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
void process_arp_packet(const struct pkb * const p)
{
  const struct arp_header * const arp = arp_hdr(p);
  print_arp_header(arp);
}
// -------------------------------------------------------------------------- //
void process_ethernet_packet(const struct pkb * const p)
{
  print_hex_bytes(p->buffer, p->length);//rx_buff_size);
//  print_hex_bytes(rx_buffer, 64);//rx_buff_size);
  const struct ethernet_header * const eth = eth_hdr(p);
  print_eth_hdr(eth);
  const uint16_t eth_type = ntohs(eth->ethertype);
  if(eth_type == ETH_ARP)
  {
    process_arp_packet(p);
  }
  else if(eth_type == ETH_IP)
  {
//    module_terminal_global_print_c_string("Got IP packet.\n");
  }
  else
  {
//    module_terminal_global_print_c_string("Got UNKNOWN packet.\n");
  }
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
void module_network_interrupt_handler(module_interrupt_registers_t x)
{
  (void)x; // avoid unused param
  uint16_t interrupt_flag = module_kernel_in_16(iobase + 0x3e);
  module_terminal_global_print_c_string("NIC IRQ flag=");
  module_terminal_global_print_uint64(interrupt_flag);
  module_terminal_global_print_c_string("\n");
  if (interrupt_flag == 0)
  {
    module_terminal_global_print_c_string("This card did not interrupt,"
      " nothing to do.\n");
    return;
  }


	if (interrupt_flag & (RX_OK | RX_ERR))
	{
		module_terminal_global_print_c_string("Packet received.\n");
	}
	else if (interrupt_flag & (TX_OK | TX_ERR))
	{
		module_terminal_global_print_c_string("Packet sent.\n");
	}

  // Acknowledge the interrupt
  module_kernel_out_16(iobase + 0x3e, interrupt_flag);

  if (!(interrupt_flag & 1))
  {
    module_terminal_global_print_c_string("This card interrupted, but there is"
      " no packet. Ack and out.\n");
    // Acknowledge the interrupt
//    module_kernel_out_16(iobase + 0x3e, interrupt_flag);
    return;
  }

  while(! rx_empty())
  {
    module_terminal_global_print_c_string("This card interrupted, and there is"
      " a packet. Processing it ...\n");
    {
//      module_terminal_global_print_c_string("rx_index=");
//      module_terminal_global_print_uint64(rx_index);
//      module_terminal_global_print_c_string("\n");
      const uint16_t * const packet_header = (const uint16_t * const)(
        rx_buffer + rx_index);
      const uint32_t flags = packet_header[0];
      const uint32_t length = packet_header[1];
//      module_terminal_global_print_c_string("PP.flags=");
//      module_terminal_global_print_uint64(flags);
//      module_terminal_global_print_c_string("\n");
//      module_terminal_global_print_c_string("PP.length=");
//      module_terminal_global_print_uint64(length);
//      module_terminal_global_print_c_string("\n");

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

//      print_hex_bytes(rx_buffer, 64);//rx_buff_size);
//      module_terminal_global_print_c_string("---\n");
      struct pkb *pk = NULL;
//      uint32_t pk_buffer = (uint32_t)(rx_buffer) + rx_index + 4;
      if ((flags & 1) == 0)
      {
        module_terminal_global_print_c_string("Got a bad packet.\n");
      } else {
        pk = (struct pkb*)module_heap_alloc(&nic_heap,
          sizeof(struct pkb) + length);
        if(pk == NULL)
        {
          module_terminal_global_print_c_string("NIC packet alloc is NULL!\n");
        }
  //      pk->from = dev;
        pk->length = length - 8;
        module_kernel_memcpy(rx_buffer + rx_index + 4, pk->buffer, pk->length);
      }
      process_ethernet_packet(pk);
      // end
      module_heap_free(&nic_heap, pk);
      rx_index += round_up(length + 4, 4);
      rx_index %= 8192;
      module_kernel_out_16(iobase + 0x38, rx_index - 16);
    }
  }

  // Acknowledge the interrupt
//  module_kernel_out_16(iobase + 0x3e, interrupt_flag);
//  module_interrupt_enable(); // needed?
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
  {
    iobase = module_pci_config_read(bus, slot, function, 0x10) & ~1;
  }
  module_terminal_global_print_c_string("NET IO base=");
  module_terminal_global_print_uint64(iobase);
  module_terminal_global_print_c_string(" = ");
  module_terminal_global_print_hex_uint64(iobase);
  module_terminal_global_print_c_string(" = ");
  module_terminal_global_print_binary_uint64(iobase);
  module_terminal_global_print_c_string("\n");

  // step 4 - get MAC address
  // you can run setting a given MAC using:
  // ~>docker exec -it os bash /mnt/run.sh && qemu-system-i386 -cdrom ~/data/h313/network/docker/research/os/build_machine/fs/project/build/bootable.iso -boot d -netdev user,id=mynet0 -device rtl8139,netdev=mynet0,mac=00:01:02:13:14:fa
  module_terminal_global_print_c_string("MAC address = ");
  struct mac_address ma;
  {
    for (uint8_t i=0; i<6; i++)
    {
      uint8_t mac_byte = module_kernel_in_8(iobase + i);
      ma.data[i] = mac_byte;
    }
    print_mac(&ma);
  }
  module_terminal_global_print_c_string(" .\n");

  // step 5 - power on
  module_terminal_global_print_c_string("Powering on the Network device ...\n");
  {
    // Send 0x00 to the CONFIG_1 register (0x52) to set the LWAKE + LWPTN to active high. this should essentially *power on* the device.
    module_kernel_out_8(iobase + 0x52, 0);
  }
  module_terminal_global_print_c_string("Network device powered on.\n");

  // step 6 - Software Reset !
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

  // step 7 - Init Receive buffer
  module_terminal_global_print_c_string("Initializing network RX buffer ...\n");
  {
    module_heap_init(&nic_heap);
    module_heap_add_block(&nic_heap, 0x110000, 1024*1024, 16);
    rx_buffer = (uint8_t*)module_heap_alloc(&nic_heap, rx_buff_size);
    if(rx_buffer == 0)
    {
      module_terminal_global_print_c_string("NIC rx_buffer= NULL !!!");
    }
    module_kernel_memset(rx_buffer, 9, rx_buff_size);
//    print_hex_bytes(rx_buffer, 64);//rx_buff_size);
    // For this part, we will send the chip a memory location to use as its receive buffer start location. One way to do it, would be to define a buffer variable and send that variables memory location to the RBSTART register (0x30).
    // Note that 'rx_buffer' needs to be a pointer to a "physical address". In this case a size of 8192 + 16 (8K + 16 bytes) is recommended, see below.
    module_kernel_out_32(iobase + 0x30, (uint32_t)(rx_buffer)); // send uint32_t memory location to RBSTART (0x30)
    module_kernel_out_32(iobase + 0x38, 0); // buffer pointer
    module_kernel_out_32(iobase + 0x3a, 0); // buffer address
  }
  module_terminal_global_print_c_string("NIC rx_buffer=");
  module_terminal_global_print_uint64((uint64_t)((uint32_t)((uint8_t*)(rx_buffer))));
  module_terminal_global_print_c_string("\n");
  module_terminal_global_print_c_string("NIC RX buff initialized.\n");

  // step 8 - enable RX & TX
  module_terminal_global_print_c_string("NIC enable RX & TX ...\n");
  {
    // 0x3c = Interrupt Mask Register
    module_kernel_out_16(iobase + 0x3c, (RX_OK | TX_OK | TX_ERR));
//    module_kernel_out_32(iobase + 0x40, 0x600); // send larger DMA bursts
    // 0x44 = Receive Config Register
    module_kernel_out_32(iobase + 0x44,
      (RCR_AAP | RCR_APM | RCR_AM | RCR_AB | RCR_WRAP)); // accept all packets + unlimited DMA
    module_kernel_out_32(iobase + 0x4c, 0x0); // RX_MISSED
    module_kernel_out_8(iobase + 0x37, 0x0c); // enable rx and tx bits high
  }
  module_terminal_global_print_c_string("NIC enabled RX & TX.\n");


  // step 3 - set IRQ handler
  {
    uint32_t irq = module_pci_config_read(bus, slot, function, 0x3c) &0xff;
    module_terminal_global_print_c_string("Read IRQ=");
    module_terminal_global_print_uint64(irq);
    module_terminal_global_print_c_string("\n");
    pic_irq_unmask(irq);

    // See : https://forum.osdev.org/viewtopic.php?f=1&t=33260
    // and:
    // https://github.com/narke/Aragveli/blob/master/src/extra/drivers/rtl8139.c
    irq += 32; // 43 is actually triggered instead of 11
    module_interrupt_register_interrupt_handler(irq,
      module_network_interrupt_handler);
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

