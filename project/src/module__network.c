// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// See: https://wiki.osdev.org/Network_Stack
//
// Run with: docker exec -it os bash /mnt/run.sh && (qemu-system-i386 -m 400M
// -cdrom ~/data/h313/network/docker/research/os/build_machine/fs/project/build
// /bootable.iso -boot d -netdev user,id=mynet0,hostfwd=tcp::5556-:22 -device
// rtl8139,netdev=mynet0,mac=00:01:02:13:14:fa
// -object filter-dump,id=f1,netdev=mynet0,file=dump.dat & ) && sleep 3 &&
// ( echo | telnet 127.0.0.1 5556 )
// -------------------------------------------------------------------------- //
#include "module__network.h"
#include "module_kernel.h"
#include "module_base.h"
#include "module_terminal.h"
#include "module_pci.h"
#include "module_heap.h"
#include "module_interrupt.h"
#include "module__driver__rtl8139.h" // recursive?
// -------------------------------------------------------------------------- //
//static const module__network__mac_address broadcast_mac =
//  {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
//static const module__network__mac_address zero_mac = {{0, 0, 0, 0, 0, 0}};
static const module__network__mac_address my_mac =
  {{0x00, 0x01, 0x02, 0x13, 0x14, 0xfa}};
// https://www.browserling.com/tools/ip-to-dec
const uint32_t my_ip = 167772687; // 10.0.2.15
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
//  for (uint8_t i=0; i<4; i++)
//  {
//    module_terminal_global_print_uint64( ((const uint8_t * const)(&ip)) [i] );
//    if(i < 3)
//    {
//      module_terminal_global_print_c_string(".");
//    }
//  }
  module_terminal_global_print_uint64( (ip) & 0xff );
  module_terminal_global_print_c_string(".");
  module_terminal_global_print_uint64( (ip >> 8) & 0xff );
  module_terminal_global_print_c_string(".");
  module_terminal_global_print_uint64( (ip >> 16) & 0xff );
  module_terminal_global_print_c_string(".");
  module_terminal_global_print_uint64( (ip >> 24) & 0xff );
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
module__network__ip_header * ip_hdr(const module__network__packet * const p)
{
  return (module__network__ip_header *)(p->buffer +
    sizeof(module__network__ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
module__network__ip__tcp_header * tcp_hdr(
  const module__network__ip_header * const ip_header)
{
  return (module__network__ip__tcp_header *)(
    ( (uint8_t *)(ip_header) ) + (ip_header->header_length * 4)
  );
}
// -------------------------------------------------------------------------- //
// ARP Packet
// -------------------------------------------------------------------------- //
void print_arp_header(const module__network__arp_header * const h)
{
  module_terminal_global_print_c_string("arp_header");
  module_terminal_global_print_c_string("{ \"hw_type\": \"");
  module_terminal_global_print_uint64(h->hw_type);
  module_terminal_global_print_c_string("\", \"proto\": \"");
  module_terminal_global_print_uint64(h->proto);
  module_terminal_global_print_c_string("\", \"hw_size\": \"");
  module_terminal_global_print_uint64(h->hw_size);
  module_terminal_global_print_c_string("\", \"proto_size\": \"");
  module_terminal_global_print_uint64(h->proto_size);
  module_terminal_global_print_c_string("\", \"operation_type\": \"");
  const uint16_t operation_type = ntohs(h->operation_type);
  if(operation_type == module__network__ethernet__arp_operation_type__request)
  {
    module_terminal_global_print_c_string("request");
  }
  else if(operation_type ==
    module__network__ethernet__arp_operation_type__response)
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
  module_terminal_global_print_c_string("\", \"sender_mac\": \"");
  module__network__print_mac(&(h->sender_mac));
  module_terminal_global_print_c_string("\", \"sender_ip\": \"");
  module__network__print_ip(h->sender_ip);
  module_terminal_global_print_c_string("\", \"target_mac\": \"");
  module__network__print_mac(&(h->target_mac));
  module_terminal_global_print_c_string("\", \"target_ip\": \"");
  module__network__print_ip(h->target_ip);
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
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

  r_arp->hw_type = htons(1); // eth_hdr
  r_arp->proto = htons(module__network__ethernet_header_type__ip); // ip_hdr
  r_arp->hw_size = 6;
  r_arp->proto_size = 4;
  r_arp->operation_type =
    htons(module__network__ethernet__arp_operation_type__response);
  r_arp->sender_mac = my_mac;
  r_arp->sender_ip = htonl(my_ip);
  r_arp->target_mac = s_arp->sender_mac;
  r_arp->target_ip = s_arp->sender_ip;
  print_arp_header(r_arp);

  response->length = sizeof(module__network__ethernet_header) +
    sizeof(module__network__arp_header);
/*
  const int ETH_MIN_LENGTH = 64;
  module_terminal_global_print_c_string("add padding=");
  module_terminal_global_print_uint64(ETH_MIN_LENGTH - response->length);
  module_terminal_global_print_c_string("\n");
  while(response->length <= ETH_MIN_LENGTH)
  {
    (response->buffer)[response->length++] = 0;
  }
  response->length -= 1;
*/
  module_terminal_global_print_c_string("length=");
  module_terminal_global_print_uint64(response->length);
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void process_arp_packet(const module__network__packet * const p)
{
  const module__network__arp_header * const arp =
    (const module__network__arp_header * const)(
    p->buffer + sizeof(module__network__ethernet_header)
  );
  print_hex_bytes(p->buffer, p->length);
  print_arp_header(arp);
  if (ntohs(arp->operation_type) ==
     module__network__ethernet__arp_operation_type__request
      && arp->target_ip == htonl(my_ip)
  )
  {
    module_terminal_global_print_c_string("Replying to ARP\n");
    module__network__packet *response_packet = new_pk();
    arp_reply(response_packet, p);
    print_hex_bytes(response_packet->buffer, response_packet->length);
    module__driver__rtl8139__send_packet(response_packet);
    free(response_packet);
  }
}
// -------------------------------------------------------------------------- //
// Transmission Control Protocol (TCP) Packet
// -------------------------------------------------------------------------- //
void print_ip_tcp_header(const module__network__ip__tcp_header * const h)
{
  module_terminal_global_print_c_string("tcp_header");
  module_terminal_global_print_c_string("{ \"source_port\": \"");
  module_terminal_global_print_uint64(h->source_port);
  module_terminal_global_print_c_string("\", \"destination_port\": \"");
  module_terminal_global_print_uint64(h->destination_port);
  module_terminal_global_print_c_string("\", \"seq\": \"");
  module_terminal_global_print_uint64(h->seq);
  module_terminal_global_print_c_string("\", \"ack\": \"");
  module_terminal_global_print_uint64(h->ack);
  module_terminal_global_print_c_string("\", \"_reserved\": \"");
  module_terminal_global_print_uint64(h->_reserved);
  module_terminal_global_print_c_string("\", \"offset\": \"");
  module_terminal_global_print_uint64(h->offset);

  module_terminal_global_print_c_string("\", \"f_fin\": \"");
  module_terminal_global_print_uint64(h->f_fin);
  module_terminal_global_print_c_string("\", \"f_syn\": \"");
  module_terminal_global_print_uint64(h->f_syn);
  module_terminal_global_print_c_string("\", \"f_rst\": \"");
  module_terminal_global_print_uint64(h->f_rst);
  module_terminal_global_print_c_string("\", \"f_psh\": \"");
  module_terminal_global_print_uint64(h->f_psh);
  module_terminal_global_print_c_string("\", \"f_ack\": \"");
  module_terminal_global_print_uint64(h->f_ack);
  module_terminal_global_print_c_string("\", \"f_urg\": \"");
  module_terminal_global_print_uint64(h->f_urg);

  module_terminal_global_print_c_string("\", \"_reserved2\": \"");
  module_terminal_global_print_uint64(h->_reserved2);
  module_terminal_global_print_c_string("\", \"window\": \"");
  module_terminal_global_print_uint64(h->window);
  module_terminal_global_print_c_string("\", \"checksum\": \"");
  module_terminal_global_print_uint64(h->checksum);
  module_terminal_global_print_c_string("\", \"urg_ptr\": \"");
  module_terminal_global_print_uint64(h->urg_ptr);
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void process_ip_tcp_packet(const module__network__packet * const p)
{
  const module__network__ip_header * const ip = ip_hdr(p);
  const module__network__ip__tcp_header * const tcp = tcp_hdr(ip);

  print_hex_bytes(p->buffer, p->length);
  print_ip_tcp_header(tcp);
/*
  if (ip->destination_ip != htonl(my_ip))
  {
    module_terminal_global_print_c_string("Got IP packet but not for my IP,"
      " ignoring.");
    return;
  }

  switch (ip->protocol)
  {
    case module__network__ethernet__ip__protocol_type__icmp:
      module_terminal_global_print_c_string("Handling IP_ICMP packet.\n");
//      echo_icmp(p);
      break;
    case module__network__ethernet__ip__protocol_type__tcp:
      module_terminal_global_print_c_string("Handling IP_TCP packet.\n");
      process_ip_tcp_packet(p);
      break;
    case module__network__ethernet__ip__protocol_type__udp:
      module_terminal_global_print_c_string("Handling IP_UDP packet.\n");
//      socket_dispatch_udp(p);
      break;
    default:
      module_terminal_global_print_c_string("Unknown IP protocol: ");
      module_terminal_global_print_uint64(ip->protocol);
      module_terminal_global_print_c_string("\n");
      break;
  }
*/
}
// -------------------------------------------------------------------------- //
// Internet Protocol (IP) Packet
// -------------------------------------------------------------------------- //
void print_ip_header(const module__network__ip_header * const h)
{
  module_terminal_global_print_c_string("ip_header");
  module_terminal_global_print_c_string("{ \"header_length\": \"");
  module_terminal_global_print_binary_uint64(h->header_length);
  module_terminal_global_print_c_string("(");
  module_terminal_global_print_uint64(h->header_length);
  module_terminal_global_print_c_string(")\", \"version\": \"");
  module_terminal_global_print_binary_uint64(h->version);
  module_terminal_global_print_c_string("(");
  module_terminal_global_print_uint64(h->header_length);
  module_terminal_global_print_c_string(")\", \"dscp\": \"");
  module_terminal_global_print_binary_uint64(h->dscp);
  module_terminal_global_print_c_string("\", \"total_length\": \"");
  module_terminal_global_print_uint64(h->total_length);
  module_terminal_global_print_c_string("\", \"id\": \"");
  module_terminal_global_print_uint64(h->id);
  module_terminal_global_print_c_string("\", \"flags_frag\": \"");
  module_terminal_global_print_binary_uint64(h->flags_frag);
  module_terminal_global_print_c_string("\", \"ttl\": \"");
  module_terminal_global_print_uint64(h->ttl);
  module_terminal_global_print_c_string("\", \"protocol\": \"");
  if(h->protocol == module__network__ethernet__ip__protocol_type__icmp)
  {
    module_terminal_global_print_c_string("ICMP");
  }
  else if(h->protocol == module__network__ethernet__ip__protocol_type__tcp)
  {
    module_terminal_global_print_c_string("TCP");
  }
  else if(h->protocol == module__network__ethernet__ip__protocol_type__udp)
  {
    module_terminal_global_print_c_string("UDP");
  }
  else
  {
    module_terminal_global_print_c_string("UNKNOWN");
  }
  module_terminal_global_print_c_string("(");
  module_terminal_global_print_uint64(h->protocol);
  module_terminal_global_print_c_string(")");
  module_terminal_global_print_c_string("\", \"header_checksum\": \"");
  module_terminal_global_print_uint64(h->header_checksum);
  module_terminal_global_print_c_string("\", \"source_ip\": \"");
  module__network__print_ip(h->source_ip);
  module_terminal_global_print_c_string("\", \"destination_ip\": \"");
  module__network__print_ip(h->destination_ip);
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void process_ip_packet(const module__network__packet * const p)
{
  const module__network__ip_header * const ip = ip_hdr(p);

  print_hex_bytes(p->buffer, p->length);
  print_ip_header(ip);
  if (ip->destination_ip != htonl(my_ip))
  {
    module_terminal_global_print_c_string("Got IP packet but not for my IP,"
      " ignoring.");
    return;
  }

  switch (ip->protocol)
  {
    case module__network__ethernet__ip__protocol_type__icmp:
      module_terminal_global_print_c_string("Handling IP_ICMP packet.\n");
//      echo_icmp(p);
      break;
    case module__network__ethernet__ip__protocol_type__tcp:
      module_terminal_global_print_c_string("Handling IP_TCP packet.\n");
      process_ip_tcp_packet(p);
      break;
    case module__network__ethernet__ip__protocol_type__udp:
      module_terminal_global_print_c_string("Handling IP_UDP packet.\n");
//      socket_dispatch_udp(p);
      break;
    default:
      module_terminal_global_print_c_string("Unknown IP protocol: ");
      module_terminal_global_print_uint64(ip->protocol);
      module_terminal_global_print_c_string("\n");
      break;
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
//  print_hex_bytes(p->buffer, p->length);//rx_buff_size);
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
    process_ip_packet(p);
  }
  else
  {
//    module_terminal_global_print_c_string("Got UNKNOWN packet.\n");
  }
}
// -------------------------------------------------------------------------- //

