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
//static const module__network__mac_address broadcast_mac =
//  {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
//static const module__network__mac_address zero_mac = {{0, 0, 0, 0, 0, 0}};
static const module__network__mac_address my_mac =
  {{0x00, 0x01, 0x02, 0x13, 0x14, 0xfa}};
// -------------------------------------------------------------------------- //
// Packet Utils functions
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
static inline uint16_t ntohs(uint16_t x)
{
  return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}
// -------------------------------------------------------------------------- //
static inline uint16_t htons(uint16_t x)
{
  return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}
// -------------------------------------------------------------------------- //
static inline uint32_t ntohl(uint32_t x)
{
  return (
    ((x & 0xFF000000) >> 24) |
    ((x & 0x00FF0000) >> 8) |
    ((x & 0x0000FF00) << 8) |
    ((x & 0x000000FF) << 24)
  );
}
// -------------------------------------------------------------------------- //
static inline uint32_t htonl(uint32_t x)
{
  return (
    ((x & 0xFF000000) >> 24) |
    ((x & 0x00FF0000) >> 8) |
    ((x & 0x0000FF00) << 8) |
    ((x & 0x000000FF) << 24)
  );
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
void module__network__print_ip(const uint32_t ip)
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
#define ETH_MTU 1536
module__network__packet * new_pk()
{
  const size_t packet_size = sizeof(module__network__packet) + ETH_MTU;
  module__network__packet * new_pk = malloc(packet_size);
  module_kernel_memset(new_pk, 1, packet_size);
  new_pk->length = -1;
//  new_pk->from = NULL;
//  new_pk->refcount = 1;
  return new_pk;
}
// -------------------------------------------------------------------------- //
module__network__ethernet_header * eth_hdr(
  const module__network__packet * const p)
{
  return (module__network__ethernet_header *)&(p->buffer);
}
// -------------------------------------------------------------------------- //
module__network__arp_header * arp_hdr(const module__network__packet * const p)
{
  return (module__network__arp_header *)(p->buffer +
    sizeof(module__network__ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
// ARP Packet
// -------------------------------------------------------------------------- //
void arp_reply(module__network__packet * response,
  const module__network__packet * const request)
{
  module__network__ethernet_header * eth = eth_hdr(response);
  const module__network__arp_header * const s_arp = arp_hdr(request);
  module__network__arp_header * r_arp = arp_hdr(response);

  eth->source_mac = my_mac;
  eth->destination_mac = s_arp->sender_mac;
  eth->ethertype = htons(module__network__ethernet_header_type__arp);

  r_arp->hw_type = htons(1);      // eth_hdr
  r_arp->proto = htons(0x0800);   // ip_hdr
  r_arp->hw_size = 6;
  r_arp->proto_size = 4;
  r_arp->operation_type =
    htons(network__ethernet__arp_operation_type__response);
  r_arp->sender_mac = my_mac;
  r_arp->sender_ip = htonl(167772687); // 10.0.2.15
  r_arp->target_mac = s_arp->sender_mac;
  r_arp->target_ip = s_arp->sender_ip;
  print_arp_header(r_arp);

  response->length = sizeof(module__network__ethernet_header) +
    sizeof(module__network__arp_header);
}
// -------------------------------------------------------------------------- //
void print_arp_header(const module__network__arp_header * const h)
{
  module_terminal_global_print_c_string("arp_header");
  module_terminal_global_print_c_string("{ \"sender_ip\": \"");
  module__network__print_ip(h->sender_ip);
  module_terminal_global_print_c_string("\", \"target_ip\": \"");
  module__network__print_ip(h->target_ip);
  module_terminal_global_print_c_string("\", \"operation_type\": \"");
  const uint16_t operation_type = ntohs(h->operation_type);
  if(operation_type == network__ethernet__arp_operation_type__request)
  {
    module_terminal_global_print_c_string("request");
  }
  else if(operation_type == network__ethernet__arp_operation_type__response)
  {
    module_terminal_global_print_c_string("response");
  }
  else
  {
    module_terminal_global_print_c_string("UNKNOWN");
  }
  module_terminal_global_print_c_string("(");
  module_terminal_global_print_uint64(operation_type);
  module_terminal_global_print_c_string(")");
  // print more header fields
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void process_arp_packet(const module__network__packet * const p)
{
  const module__network__arp_header * const arp =
    (const module__network__arp_header * const)(
    p->buffer + sizeof(module__network__ethernet_header)
  );
  print_arp_header(arp);
  if (ntohs(arp->operation_type) ==
     network__ethernet__arp_operation_type__request
//      && arp->target_ip == pk->from->ip
  )
  {
    module__network__packet *response_packet = new_pk();
    arp_reply(response_packet, p);
//    pk->from->drv->send(pk->from, resp);
    free(response_packet);
  }
}
// -------------------------------------------------------------------------- //
// Ethernet Packet
// -------------------------------------------------------------------------- //
void print_eth_hdr(const module__network__ethernet_header * const h)
{
  module_terminal_global_print_c_string("ethernet_header");
  module_terminal_global_print_c_string("{ \"source_mac\": \"");
  module__network__print_mac(&(h->source_mac));
  module_terminal_global_print_c_string("\", \"destination_mac\": \"");
  module__network__print_mac(&(h->destination_mac));
  module_terminal_global_print_c_string("\", \"type\": \"");
  const uint16_t eth_type = ntohs(h->ethertype);
  if(eth_type == module__network__ethernet_header_type__arp)
  {
    module_terminal_global_print_c_string("ARP");
  }
  else if(eth_type == module__network__ethernet_header_type__ip)
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
void module__network__process_ethernet_packet(
  const module__network__packet * const p)
{
  print_hex_bytes(p->buffer, p->length);//rx_buff_size);
//  print_hex_bytes(rx_buffer, 64);//rx_buff_size);
  const module__network__ethernet_header * const eth = eth_hdr(p);
  print_eth_hdr(eth);
  const uint16_t eth_type = ntohs(eth->ethertype);
  if(eth_type == module__network__ethernet_header_type__arp)
  {
    process_arp_packet(p);
  }
  else if(eth_type == module__network__ethernet_header_type__ip)
  {
//    module_terminal_global_print_c_string("Got IP packet.\n");
  }
  else
  {
//    module_terminal_global_print_c_string("Got UNKNOWN packet.\n");
  }
}
// -------------------------------------------------------------------------- //

