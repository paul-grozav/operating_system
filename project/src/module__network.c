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
void tcp_checksum(module__network__packet *p)
{
  module__network__ip_header *ip = ip_hdr(p);
  module__network__ip__tcp_header *tcp = tcp_hdr(ip);

  int length = ntohs(ip->total_length);
  int n_bytes = length - sizeof(module__network__ip_header);

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
    module__network__ethernet__ip__protocol_type__tcp,
    htons(n_bytes),
  };

  uint32_t sum = 0;
  uint16_t *c = (uint16_t *)&t;
  for (size_t i=0; i<sizeof(t)/2; i++)
  {
    sum += c[i];
  }

  c = (uint16_t *)tcp;
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
}
// -------------------------------------------------------------------------- //
void print_ip_tcp_header(const module__network__ip__tcp_header * const h)
{
  module_terminal_global_print_c_string("tcp_header");
  module_terminal_global_print_c_string("{ \"source_port\": \"");
  module_terminal_global_print_uint64(ntohs(h->source_port));
  module_terminal_global_print_c_string("\", \"destination_port\": \"");
  module_terminal_global_print_uint64(ntohs(h->destination_port));
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
// declare ip_checksum - it is defined below, in IP section, but used here
void ip_checksum(module__network__packet *p);
void make_ip_tcp_packet(module__network__packet *p,
  const uint32_t destination_ip, const uint16_t destination_port,
  const uint16_t source_port, const int flags, const void * const data,
  const size_t len)
{
  module__network__ethernet_header *eth = eth_hdr(p);
  eth->ethertype = htons(module__network__ethernet_header_type__ip);

  module__network__ip_header *ip = ip_hdr(p);
  ip->version = 4;
  ip->header_length = 5;
  ip->dscp = 0;
  ip->id = ntohs(1); //s->ip_id);
  ip->flags_frag = htons(0x4000); // DNF
  ip->ttl = 64;
  ip->protocol = module__network__ethernet__ip__protocol_type__tcp;
  ip->source_ip = htonl(my_ip); //s->local_ip;
  ip->destination_ip = destination_ip;
  ip->total_length = htons(sizeof(module__network__ip_header) +
    sizeof(module__network__ip__tcp_header) + len);

  module__network__ip__tcp_header *tcp = tcp_hdr(ip);
  tcp->source_port = source_port; //54321;//s->local_port;
  tcp->destination_port = destination_port; //80;//s->remote_port;
  tcp->seq = htonl(3768610427);//0); //s->send_seq);
  if (flags & module__network__ip__tcp_flag__ack)
  {
    tcp->ack = htonl(1); //s->recv_seq);
  }
  else
  {
    tcp->ack = 0;
  }
  tcp->f_ns = 0;
  tcp->_reserved = 0;
  tcp->offset = 10/2;//5; // 5=no_opt & 10=opt
  tcp->f_fin = ((flags & module__network__ip__tcp_flag__fin) > 0);
  tcp->f_syn = ((flags & module__network__ip__tcp_flag__syn) > 0);
  tcp->f_rst = ((flags & module__network__ip__tcp_flag__rst) > 0);
  tcp->f_psh = ((flags & module__network__ip__tcp_flag__psh) > 0);
  tcp->f_ack = ((flags & module__network__ip__tcp_flag__ack) > 0);
  tcp->f_urg = ((flags & module__network__ip__tcp_flag__urg) > 0);
  tcp->f_ece = 0;
  tcp->f_cwr = 0;
  tcp->window = htons(64240);//0x1000);
  tcp->checksum = 0;
  tcp->urg_ptr = 0;

  module_kernel_memcpy(data, tcp->data, len);

  p->length = ntohs(ip->total_length)
    + sizeof(module__network__ethernet_header);

  tcp_checksum(p);
  ip_checksum(p);
}
// -------------------------------------------------------------------------- //
void process_ip_tcp_packet(const module__network__packet * const p)
{
  const module__network__ip_header * const ip = ip_hdr(p);
  const module__network__ip__tcp_header * const tcp = tcp_hdr(ip);

  print_hex_bytes(p->buffer, p->length);
  print_ip_tcp_header(tcp);

  module_terminal_global_print_c_string("Processing IP_TCP packet\n");
//  module__network__test2();
}
// -------------------------------------------------------------------------- //
// Internet Protocol (IP) Packet
// -------------------------------------------------------------------------- //
void ip_checksum(module__network__packet *p)
{
  module__network__ip_header *ip = ip_hdr(p);

  uint16_t *ip_chunks = (uint16_t *)ip;
  uint32_t checksum32 = 0;
  for (int i=0; i<ip->header_length*2; i+=1)
  {
    checksum32 += ip_chunks[i];
  }
  uint16_t checksum = (checksum32 & 0xFFFF) + (checksum32 >> 16);

  ip->header_checksum = ~checksum;
}
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
void module__network__test2()
{
  module_terminal_global_print_c_string("Sending IP_TCP packet\n");
  module__network__packet *response = new_pk();
  uint32_t dst_ip = 0;
//  dst_ip = 3627734734; // = 216.58.214.206 = google.com
//  dst_ip = 3232286790; // 192.168.200.70 = host
  dst_ip = 167772674; // 10.0.2.2 = GW
  dst_ip = htonl(dst_ip);
  uint16_t dst_port = htons(1030); // web server
  make_ip_tcp_packet(response, dst_ip, dst_port,
    htons(9876),
    (module__network__ip__tcp_flag__syn),
    "\0", 1);//"\x02\x04\x05\xb4\x04\x02\x08\x0a\x31\xe6\xe7\x6c\x0\x0\x0\x0\x01\x03\x03\x07", 20);
  print_hex_bytes(response->buffer, response->length);
  const module__network__ip_header * const response_ip = ip_hdr(response);
  const module__network__ip__tcp_header * const response_tcp =
    tcp_hdr(response_ip);
  print_ip_tcp_header(response_tcp);
  module__driver__rtl8139__send_packet(response);
  free(response);
}