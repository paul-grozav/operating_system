// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module__network.h"
#include "module_kernel.h"
#include "module_base.h"
#include "module_terminal.h"
#include "module_pci.h"
#include "module_heap.h"
#include "module_interrupt.h"
// -------------------------------------------------------------------------- //
// -----
typedef uint32_t be32;
typedef uint16_t be16;

#define __const_htons(x) ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8));
#define __const_ntohs __const_htons
// -------------------------------------------------------------------------- //
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
// -------------------------------------------------------------------------- //
//static const module__network__mac_address broadcast_mac =
//  {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
//static const module__network__mac_address zero_mac = {{0, 0, 0, 0, 0, 0}};
// -------------------------------------------------------------------------- //
void module__network__print_mac(const module__network__mac_address * const ma)
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
// -------------------------------------------------------------------------- //
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
// -------------------------------------------------------------------------- //
struct __attribute__((__packed__)) ethernet_header
{
  module__network__mac_address destination_mac;
  module__network__mac_address source_mac;
  be16 ethertype;
  char data[];
};
// -------------------------------------------------------------------------- //
enum arp_op {
    ARP_REQ = 1,
    ARP_RESP = 2,
};
// -------------------------------------------------------------------------- //
struct __attribute__((__packed__)) arp_header {
    // eth_hdr
    be16 hw_type;
    be16 proto;
    uint8_t hw_size;
    uint8_t proto_size;
    be16 op;
    module__network__mac_address sender_mac;
    be32 sender_ip;
    module__network__mac_address target_mac;
    be32 target_ip;
};
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
const struct ethernet_header * eth_hdr(const module__network__packet * const pk)
{
  return (const struct ethernet_header * const)&(pk->buffer);
}
// -------------------------------------------------------------------------- //
void print_eth_hdr(const struct ethernet_header * const h)
{
  module_terminal_global_print_c_string("ethernet_header");
  module_terminal_global_print_c_string("{ \"source_mac\": \"");
  module__network__print_mac(&(h->source_mac));
  module_terminal_global_print_c_string("\", \"destination_mac\": \"");
  module__network__print_mac(&(h->destination_mac));
  module_terminal_global_print_c_string("\", \"type\": \"");
  const uint16_t eth_type = ntohs(h->ethertype);
  if(eth_type == module__network__ethernet_packet_type__arp)
  {
    module_terminal_global_print_c_string("ARP");
  }
  else if(eth_type == module__network__ethernet_packet_type__ip)
  {
    module_terminal_global_print_c_string("IP");
  }
  else
  {
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
  module__network__print_mac(&(h->destination_mac));
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
const struct arp_header * arp_hdr(const module__network__packet * const pk)
{
  return (const struct arp_header * const)(
    pk->buffer + sizeof(struct ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
void process_arp_packet(const module__network__packet * const p)
{
  const struct arp_header * const arp = arp_hdr(p);
  print_arp_header(arp);
}
// -------------------------------------------------------------------------- //
void module__network__process_ethernet_packet(
  const module__network__packet * const p)
{
  print_hex_bytes(p->buffer, p->length);//rx_buff_size);
//  print_hex_bytes(rx_buffer, 64);//rx_buff_size);
  const struct ethernet_header * const eth = eth_hdr(p);
  print_eth_hdr(eth);
  const uint16_t eth_type = ntohs(eth->ethertype);
  if(eth_type == module__network__ethernet_packet_type__arp)
  {
    process_arp_packet(p);
  }
  else if(eth_type == module__network__ethernet_packet_type__ip)
  {
//    module_terminal_global_print_c_string("Got IP packet.\n");
  }
  else
  {
//    module_terminal_global_print_c_string("Got UNKNOWN packet.\n");
  }
}
// -------------------------------------------------------------------------- //

