// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include "module__network__data.h"
#include "module_terminal.h"
#include "module_base.h"
#include "module_kernel.h"
#include "module_heap.h"
// -------------------------------------------------------------------------- //
const module__network__data__mac_address
  module__network__data__mac_address__broadcast_mac =
  {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
// -------------------------------------------------------------------------- //
const module__network__data__mac_address
  module__network__data__mac_address__zero_mac = {{0, 0, 0, 0, 0, 0}};
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Utils
// -------------------------------------------------------------------------- //
uint16_t module__network__data__ntohs(const uint16_t x)
{
  return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}
// -------------------------------------------------------------------------- //
uint16_t module__network__data__htons(const uint16_t x)
{
  return (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8));
}
// -------------------------------------------------------------------------- //
uint32_t module__network__data__ntohl(const uint32_t x)
{
  return (
    ((x & 0xFF000000) >> 24) |
    ((x & 0x00FF0000) >> 8) |
    ((x & 0x0000FF00) << 8) |
    ((x & 0x000000FF) << 24)
  );
}
// -------------------------------------------------------------------------- //
uint32_t module__network__data__htonl(const uint32_t x)
{
  return (
    ((x & 0xFF000000) >> 24) |
    ((x & 0x00FF0000) >> 8) |
    ((x & 0x0000FF00) << 8) |
    ((x & 0x000000FF) << 24)
  );
}
// -------------------------------------------------------------------------- //
uint32_t module__network__data__ip(const uint8_t a, const uint8_t b,
  const uint8_t c, const uint8_t d)
{
  return (
    a <<  0 |
    b <<  8 |
    c << 16 |
    d << 24
  );
}
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Packet
// -------------------------------------------------------------------------- //
module__network__data__packet * module__network__data__packet__alloc()
{
  const size_t ETH_MTU = 1536;
  const size_t packet_size = sizeof(module__network__data__packet) + ETH_MTU;
  module__network__data__packet * new_pk = malloc(packet_size);
  module_kernel_memset(new_pk, 1, packet_size);
  new_pk->length = -1;
//  new_pk->length = packet_size;
//  new_pk->from = NULL;
//  new_pk->refcount = 1;
  return new_pk;
}
// -------------------------------------------------------------------------- //
module__network__data__packet * module__network__data__packet__create_with_data(
  const char * const data, const size_t length)
{
  const size_t packet_size = sizeof(module__network__data__packet) + length;
  module__network__data__packet * new_pk = malloc(packet_size);
  module_kernel_memcpy(data, new_pk->buffer, length);
  new_pk->length = (int32_t)(length);
  return new_pk;
}
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Ethernet header
// -------------------------------------------------------------------------- //
void module__network__data__print_mac(
  const module__network__data__mac_address * const ma)
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
module__network__data__ethernet_header *
  module__network__data__packet_get_ethernet_header(
  module__network__data__packet * const p)
{
  return (module__network__data__ethernet_header *)&(p->buffer);
}
// -------------------------------------------------------------------------- //
const module__network__data__ethernet_header *
  module__network__data__packet_get_ethernet_header_const(
  const module__network__data__packet * const p)
{
  return (const module__network__data__ethernet_header *)&(p->buffer);
}
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ethernet_header(
  const module__network__data__ethernet_header * const h)
{
  module_terminal_global_print_c_string("ethernet_header");
  module_terminal_global_print_c_string("{ \"source_mac\": \"");
  module__network__data__print_mac(&(h->source_mac));
  module_terminal_global_print_c_string("\", \"destination_mac\": \"");
  module__network__data__print_mac(&(h->destination_mac));
  module_terminal_global_print_c_string("\", \"type\": \"");
  const uint16_t eth_type = module__network__data__ntohs(h->ethertype);
  if(eth_type == module__network__data__ethernet_header_type__arp)
  {
    module_terminal_global_print_c_string("ARP");
  }
  else if(eth_type == module__network__data__ethernet_header_type__ip_v4)
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





// -------------------------------------------------------------------------- //
// Address Resolution Protocol (ARP) Packet
// -------------------------------------------------------------------------- //
module__network__data__arp_header *
  module__network__data__packet_get_arp_header(
  module__network__data__packet * const p)
{
  return (module__network__data__arp_header *)(p->buffer +
    sizeof(module__network__data__ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
const module__network__data__arp_header *
  module__network__data__packet_get_arp_header_const(
  const module__network__data__packet * const p)
{
  return (const module__network__data__arp_header *)(p->buffer +
    sizeof(module__network__data__ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
void module__network__data__arp__header_print(
  const module__network__data__arp_header * const h)
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
  const uint16_t operation_type = module__network__data__ntohs(
    h->operation_type);
  if(operation_type ==
    module__network__data__ethernet__arp_operation_type__request)
  {
    module_terminal_global_print_c_string("request");
  }
  else if(operation_type ==
    module__network__data__ethernet__arp_operation_type__response)
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
  module__network__data__print_mac(&(h->sender_mac));
  module_terminal_global_print_c_string("\", \"sender_ip\": \"");
  module__network__data__print_ipv4(h->sender_ip);
  module_terminal_global_print_c_string("\", \"target_mac\": \"");
  module__network__data__print_mac(&(h->target_mac));
  module_terminal_global_print_c_string("\", \"target_ip\": \"");
  module__network__data__print_ipv4(h->target_ip);
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Internet Protocol (IP) Packet
// -------------------------------------------------------------------------- //
void module__network__data__print_ipv4(const uint32_t ip)
{
  module_terminal_global_print_uint64( (ip) & 0xff );
  module_terminal_global_print_c_string(".");
  module_terminal_global_print_uint64( (ip >> 8) & 0xff );
  module_terminal_global_print_c_string(".");
  module_terminal_global_print_uint64( (ip >> 16) & 0xff );
  module_terminal_global_print_c_string(".");
  module_terminal_global_print_uint64( (ip >> 24) & 0xff );
}
// -------------------------------------------------------------------------- //
module__network__data__ip_header * module__network__data__packet_get_ip_header(
  module__network__data__packet * const p)
{
  return (module__network__data__ip_header *)(p->buffer +
    sizeof(module__network__data__ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
const module__network__data__ip_header *
  module__network__data__packet_get_ip_header_const(
  const module__network__data__packet * const p)
{
  return (const module__network__data__ip_header *)(p->buffer +
    sizeof(module__network__data__ethernet_header)
  );
}
// -------------------------------------------------------------------------- //
void module__network__data__ip__checksum(module__network__data__packet *p)
{
  module__network__data__ip_header *ip =
    module__network__data__packet_get_ip_header(p);
  ip->header_checksum = 0; // RFC 791 says that "For purposes of
  // computing the checksum, the value of the checksum field is zero."


  // Disable warning:
  // warning: converting a packed 'module__network__data__ip_header' {aka
  // 'struct <anonymous>'} pointer (alignment 1) to a 'uint16_t' {aka 'short
  // unsigned int'} pointer (alignment 2) may result in an unaligned pointer
  // value [-Waddress-of-packed-member]
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  uint16_t *ip_chunks = (uint16_t *)ip;
  #pragma GCC diagnostic pop
  uint32_t checksum32 = 0;
  for (int i=0; i<ip->header_length*2; i+=1)
  {
    checksum32 += ip_chunks[i];
  }
  uint16_t checksum = (checksum32 & 0xFFFF) + (checksum32 >> 16);

  ip->header_checksum = ~checksum;
}
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_header(
  const module__network__data__ip_header * const h)
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
  if(h->protocol == module__network__data__ethernet__ip__protocol_type__icmp)
  {
    module_terminal_global_print_c_string("ICMP");
  }
  else if(h->protocol ==
    module__network__data__ethernet__ip__protocol_type__tcp)
  {
    module_terminal_global_print_c_string("TCP");
  }
  else if(h->protocol ==
    module__network__data__ethernet__ip__protocol_type__udp)
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
  module__network__data__print_ipv4(h->source_ip);
  module_terminal_global_print_c_string("\", \"destination_ip\": \"");
  module__network__data__print_ipv4(h->destination_ip);
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// User Datagram Protocol (UDP)
// -------------------------------------------------------------------------- //
module__network__data__ip__udp_header *
  module__network__data__packet_get_ip_udp_header(
  module__network__data__ip_header * const ip_header)
{
  return (module__network__data__ip__udp_header *)(
    ( (uint8_t *)(ip_header) ) + (ip_header->header_length * 4)
  );
}
// -------------------------------------------------------------------------- //
const module__network__data__ip__udp_header *
  module__network__data__packet_get_ip_udp_header_const(
  const module__network__data__ip_header * const ip_header)
{
  return (const module__network__data__ip__udp_header *)(
    ( (const uint8_t *)(ip_header) ) + (ip_header->header_length * 4)
  );
}
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_udp_header(
  const module__network__data__ip__udp_header * const h)
{
  module_terminal_global_print_c_string("udp_header");
  module_terminal_global_print_c_string("{ \"source_port\": \"");
  module_terminal_global_print_uint64(module__network__data__ntohs(
    h->source_port));
  module_terminal_global_print_c_string("\", \"destination_port\": \"");
  module_terminal_global_print_uint64(module__network__data__ntohs(
    h->destination_port));
  module_terminal_global_print_c_string("\", \"length\": \"");
  module_terminal_global_print_uint64(module__network__data__ntohs(h->length));
  module_terminal_global_print_c_string("\", \"checksum\": \"");
  module_terminal_global_print_uint64(
    module__network__data__ntohs(h->checksum));
  module_terminal_global_print_c_string("\" }");
  module_terminal_global_print_c_string("\n");
}
// -------------------------------------------------------------------------- //
void module__network__data__packet_udp_checksum(
  module__network__data__packet *p)
{
  module__network__data__ip_header *ip =
    module__network__data__packet_get_ip_header(p);
  module__network__data__ip__udp_header *udp =
    module__network__data__packet_get_ip_udp_header(ip);
  udp->checksum = 0; // RFC 791 says that "For purposes of
  // computing the checksum, the value of the checksum field is zero."


  uint16_t length = module__network__data__ntohs(ip->total_length);
  uint16_t n_bytes = length - sizeof(module__network__data__ip_header);

  struct udp_pseudoheader
  {
    uint32_t source_ip;
    uint32_t destination_ip;
    uint8_t _zero;
    uint8_t protocol;
    uint16_t tcp_length;
  };
  struct udp_pseudoheader t =
  {
    ip->source_ip,
    ip->destination_ip,
    0,
    module__network__data__ethernet__ip__protocol_type__udp,
    module__network__data__htons(n_bytes),
  };

  uint32_t sum = 0;
  uint16_t *c = (uint16_t *)&t;
  for (size_t i=0; i<sizeof(t)/2; i++)
  {
    sum += c[i];
  }

  // Disable warning for this line:
  // warning: converting a packed 'module__network__data__ip__udp_header' {aka
  // 'struct <anonymous>'} pointer (alignment 1) to a 'uint16_t' {aka 'short
  // unsigned int'} pointer (alignment 2) may result in an unaligned pointer
  // value [-Waddress-of-packed-member]
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  c = (uint16_t *)udp;
  #pragma GCC diagnostic pop
  for (int i=0; i<n_bytes/2; i++)
  {
    sum += c[i];
  }

  if (n_bytes % 2 != 0)
  {
    uint16_t last = ((uint8_t *)ip)[length-1];
    sum += last;
  }

  while (sum >> 16)
  {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  udp->checksum = ~(uint16_t)sum;
//  udp->checksum = 0x77c1;
}
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Transmission Control Protocol (TCP) Packet
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_tcp_header(
  const module__network__data__ip__tcp_header * const h)
{
  module_terminal_global_print_c_string("tcp_header");
  module_terminal_global_print_c_string("{ \"source_port\": \"");
  module_terminal_global_print_uint64(module__network__data__ntohs(
    h->source_port));
  module_terminal_global_print_c_string("\", \"destination_port\": \"");
  module_terminal_global_print_uint64(module__network__data__ntohs(
    h->destination_port));
  module_terminal_global_print_c_string("\", \"seq\": \"");
  module_terminal_global_print_uint64(h->seq);
  module_terminal_global_print_c_string("\", \"ack\": \"");
  module_terminal_global_print_uint64(h->ack);
  module_terminal_global_print_c_string("\", \"f_ns\": \"");
  module_terminal_global_print_uint64(h->f_ns);
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
  module_terminal_global_print_c_string("\", \"f_ece\": \"");
  module_terminal_global_print_uint64(h->f_ece);
  module_terminal_global_print_c_string("\", \"f_cwr\": \"");
  module_terminal_global_print_uint64(h->f_cwr);

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
module__network__data__ip__tcp_header *
  module__network__data__packet_get_ip_tcp_header(
  module__network__data__ip_header * const ip_header)
{
  return (module__network__data__ip__tcp_header *)(
    ( (uint8_t *)(ip_header) ) + (ip_header->header_length * 4)
  );
}
// -------------------------------------------------------------------------- //
const module__network__data__ip__tcp_header *
  module__network__data__packet_get_ip_tcp_header_const(
  const module__network__data__ip_header * const ip_header)
{
  return (const module__network__data__ip__tcp_header *)(
    ( (const uint8_t *)(ip_header) ) + (ip_header->header_length * 4)
  );
}
// -------------------------------------------------------------------------- //
void module__network__data__packet_tcp_checksum(
  module__network__data__packet *p)
{
  module__network__data__ip_header *ip =
    module__network__data__packet_get_ip_header(p);
  module__network__data__ip__tcp_header *tcp =
    module__network__data__packet_get_ip_tcp_header(ip);

  uint16_t length = module__network__data__ntohs(ip->total_length);
  uint16_t n_bytes = length - sizeof(module__network__data__ip_header);

  struct tcp_pseudoheader
  {
    uint32_t source_ip;
    uint32_t destination_ip;
    uint8_t _zero;
    uint8_t protocol;
    uint16_t tcp_length;
  };
  struct tcp_pseudoheader t =
  {
    ip->source_ip,
    ip->destination_ip,
    0,
    module__network__data__ethernet__ip__protocol_type__tcp,
    module__network__data__htons(n_bytes),
  };

  uint32_t sum = 0;
  uint16_t *c = (uint16_t *)&t;
  for (size_t i=0; i<sizeof(t)/2; i++)
  {
    sum += c[i];
  }

  // Disable warning for this line:
  // warning: converting a packed 'module__network__data__ip__tcp_header' {aka
  // 'struct <anonymous>'} pointer (alignment 1) to a 'uint16_t' {aka 'short
  // unsigned int'} pointer (alignment 2) may result in an unaligned pointer
  // value [-Waddress-of-packed-member]
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  c = (uint16_t *)tcp;
  #pragma GCC diagnostic pop
  for (int i=0; i<n_bytes/2; i++)
  {
    sum += c[i];
  }

  if (n_bytes % 2 != 0)
  {
    uint16_t last = ((uint8_t *)ip)[length-1];
    sum += last;
  }

  while (sum >> 16)
  {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  tcp->checksum = ~(uint16_t)sum;
//  tcp->checksum = 0x77c1;
}





// -------------------------------------------------------------------------- //
// Bootstrap Protocol (BOOTP)
// -------------------------------------------------------------------------- //
module__network__data__ip__udp__bootp_header *
  module__network__data__packet_get_ip_udp_bootp_header(
  module__network__data__ip__udp_header * const udp_header)
{
  return (module__network__data__ip__udp__bootp_header *)(udp_header->data);
}
// -------------------------------------------------------------------------- //
const module__network__data__ip__udp__bootp_header *
  module__network__data__packet_get_ip_udp_bootp_header_const(
  const module__network__data__ip__udp_header * const udp_header)
{
  return
    (const module__network__data__ip__udp__bootp_header *)(udp_header->data);
}
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_udp_bootp_header(
  const module__network__data__ip__udp__bootp_header * const h)
{
  module_terminal_global_print_c_string("bootp_header");
  module_terminal_global_print_c_string("{ \"op_code\": \"");
  module_terminal_global_print_uint64(h->op_code);
  module_terminal_global_print_c_string("\", \"hardware_address_type\": \"");
  module_terminal_global_print_uint64(h->hardware_address_type);
  module_terminal_global_print_c_string("\", \"hardware_address_length\": \"");
  module_terminal_global_print_uint64(h->hardware_address_length);
  module_terminal_global_print_c_string("\", \"gateway_hops\": \"");
  module_terminal_global_print_uint64(h->gateway_hops);
  module_terminal_global_print_c_string("\", \"transaction_id\": \"");
  module_terminal_global_print_uint64(
    module__network__data__ntohl(h->transaction_id));
  module_terminal_global_print_c_string("\", \"seconds_since_boot\": \"");
  module_terminal_global_print_uint64(h->seconds_since_boot);
  module_terminal_global_print_c_string("\", \"flags\": \"");
  module_terminal_global_print_uint64(h->flags);
  module_terminal_global_print_c_string("\", \"client_ip_address\": \"");
  module__network__data__print_ipv4(h->client_ip_address);
  module_terminal_global_print_c_string("\", \"your_ip_address\": \"");
  module__network__data__print_ipv4(h->your_ip_address);
  module_terminal_global_print_c_string("\", \"server_ip_address\": \"");
  module__network__data__print_ipv4(h->server_ip_address);
  module_terminal_global_print_c_string("\", \"gateway_ip_address\": \"");
  module__network__data__print_ipv4(h->gateway_ip_address);
  module_terminal_global_print_c_string("\", \"client_hardware_address\": \"");
  module__network__data__print_mac(
    (const module__network__data__mac_address*)(h->client_hardware_address));
  module_terminal_global_print_c_string("\", \"server_host_name\": \"");
  module_terminal_global_print_c_string((const char*)(h->server_host_name));
  module_terminal_global_print_c_string("\", \"boot_file_name\": \"");
  module_terminal_global_print_c_string((const char*)(h->boot_file_name));
  module_terminal_global_print_c_string("\", \"vendor_area\": \"");
  module_terminal_global_print_c_string((const char*)(h->vendor_area));
  module_terminal_global_print_c_string("\" }\n");
}
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
