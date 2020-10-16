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
#include "module__pci.h"
#include "module_heap.h"
#include "module_interrupt.h"
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
#include "module__network__ip__tcp.h"
#include "module__network__service__dhcp__client.h"
#include "module__network__service__http__client.h"
#include "module__driver__rtl8139.h" // recursive?
// -------------------------------------------------------------------------- //
// https://www.browserling.com/tools/ip-to-dec
//const uint32_t my_ip = 167772687; // 10.0.2.15
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// Packet Utils functions
// -------------------------------------------------------------------------- //
//static inline uintptr_t round_down(uintptr_t val, uintptr_t place)
//{
//  return val & ~(place - 1);
//}
// -------------------------------------------------------------------------- //
//static inline uintptr_t round_up(uintptr_t val, uintptr_t place)
//{
//  return round_down(val + place - 1, place);
//}
// -------------------------------------------------------------------------- //
// ARP Packet
// -------------------------------------------------------------------------- //
void arp_query(module__network__data__packet * request, uint32_t address,
  const module__network__ethernet_interface * const interface)
{
  module__network__data__ethernet_header * eth =
    module__network__data__packet_get_ethernet_header(request);
  eth->destination_mac = module__network__data__mac_address__broadcast_mac;
  eth->source_mac = interface->mac_address;
  eth->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__arp);

  module__network__data__arp_header * r_arp =
    module__network__data__packet_get_arp_header(request);
  r_arp->hw_type = module__network__data__htons(1);      // eth_hdr
  r_arp->proto = module__network__data__htons(0x0800);   // ip_hdr
  r_arp->hw_size = 6;
  r_arp->proto_size = 4;
  r_arp->operation_type = module__network__data__htons(
    module__network__data__ethernet__arp_operation_type__request);
  r_arp->sender_mac = interface->mac_address;
  r_arp->sender_ip = module__network__data__htonl(interface->ip);
  r_arp->target_mac = module__network__data__mac_address__zero_mac;
  r_arp->target_ip = module__network__data__htonl(address);

  request->length = sizeof(module__network__data__ethernet_header) +
    sizeof(module__network__data__arp_header);
}
// -------------------------------------------------------------------------- //
void arp_reply(module__network__data__packet * response,
  const module__network__data__packet * const request,
  const module__network__ethernet_interface * const interface)
{
  module__network__data__ethernet_header * eth =
    module__network__data__packet_get_ethernet_header(response);
  const module__network__data__arp_header * const s_arp =
    module__network__data__packet_get_arp_header_const(request);
  module__network__data__arp_header * r_arp =
    module__network__data__packet_get_arp_header(response);

  eth->source_mac = interface->mac_address;
  eth->destination_mac = s_arp->sender_mac;
  eth->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__arp);

  r_arp->hw_type = module__network__data__htons(1); // eth_hdr
  r_arp->proto = module__network__data__htons(
    module__network__data__ethernet_header_type__ip_v4);
  r_arp->hw_size = 6;
  r_arp->proto_size = 4;
  r_arp->operation_type = module__network__data__htons(
    module__network__data__ethernet__arp_operation_type__response);
  r_arp->sender_mac = interface->mac_address;
  r_arp->sender_ip = module__network__data__htonl(interface->ip);
  r_arp->target_mac = s_arp->sender_mac;
  r_arp->target_ip = s_arp->sender_ip;
  module__network__data__arp__header_print(r_arp);

  response->length = sizeof(module__network__data__ethernet_header) +
    sizeof(module__network__data__arp_header);
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
void process_arp_packet(const module__network__data__packet * const p,
  module__network__ethernet_interface * const interface)
{
  const module__network__data__arp_header * const arp =
    module__network__data__packet_get_arp_header_const(p);
  module_terminal_print_buffer_hex_bytes(p->buffer, p->length);
  module__network__data__arp__header_print(arp);
  if (module__network__data__ntohs(arp->operation_type) ==
     module__network__data__ethernet__arp_operation_type__request
      && arp->target_ip == module__network__data__htonl(interface->ip)
  )
  {
    module_terminal_global_print_c_string("Replying to ARP\n");
    module__network__data__packet *response_packet =
      module__network__data__packet__alloc();
    arp_reply(response_packet, p, interface);
    module_terminal_print_buffer_hex_bytes(response_packet->buffer,
      response_packet->length);
    module__network__ethernet_interface__send_packet(response_packet,
      interface);
    free(response_packet);
  }
}















// -------------------------------------------------------------------------- //
// Transmission Control Protocol (TCP) Packet
// -------------------------------------------------------------------------- //
void make_ip_tcp_packet(module__network__data__packet *p,
  const uint32_t destination_ip, const uint16_t destination_port,
  const uint16_t source_port, const int flags, const void * const data,
  const size_t len, const module__network__ethernet_interface * const interface)
{
  module__network__data__ethernet_header *eth =
    module__network__data__packet_get_ethernet_header(p);
  eth->source_mac = interface->mac_address;

  static const module__network__data__mac_address dest_mac =
    {{0x52, 0x55, 0x0a, 0x0, 0x02, 0x02}};
  eth->destination_mac = dest_mac;
  eth->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__ip_v4);

  module__network__data__ip_header *ip =
    module__network__data__packet_get_ip_header(p);
  ip->version = 4;
  ip->header_length = 5;
  ip->dscp = 0;
  ip->id = module__network__data__ntohs(45715);//);//1); //s->ip_id);
  ip->flags_frag = module__network__data__htons(0x4000);//0);//0x4000); // DNF
  ip->ttl = 64;
  ip->protocol = module__network__data__ethernet__ip__protocol_type__tcp;
  ip->source_ip = module__network__data__htonl(interface->ip);
  ip->destination_ip = destination_ip;
  ip->total_length = module__network__data__htons(
    sizeof(module__network__data__ip_header) +
    sizeof(module__network__data__ip__tcp_header) + len);

  module__network__data__ip__tcp_header *tcp =
    module__network__data__packet_get_ip_tcp_header(ip);
  tcp->source_port = source_port; //54321;//s->local_port;
  tcp->destination_port = destination_port; //80;//s->remote_port;
  tcp->seq = module__network__data__htonl(0xce815259);//384001);//3768610427);//0); //s->send_seq);
  if (flags & module__network__data__ip__tcp_flag__ack)
  {
    tcp->ack = module__network__data__htonl(0);//1); //s->recv_seq);
  }
  else
  {
    tcp->ack = 0;
  }
  tcp->f_ns = 0; // 1 bit
  tcp->_reserved = 0; // 3 bits
  tcp->offset = 10;//6;//10/2;//5; // 5=no_opt & 10=opt // 4 bits
  tcp->f_fin = ((flags & module__network__data__ip__tcp_flag__fin) > 0);
  tcp->f_syn = ((flags & module__network__data__ip__tcp_flag__syn) > 0);
  tcp->f_rst = ((flags & module__network__data__ip__tcp_flag__rst) > 0);
  tcp->f_psh = ((flags & module__network__data__ip__tcp_flag__psh) > 0);
  tcp->f_ack = ((flags & module__network__data__ip__tcp_flag__ack) > 0);
  tcp->f_urg = ((flags & module__network__data__ip__tcp_flag__urg) > 0);
  tcp->f_ece = 0;
  tcp->f_cwr = 0;
  tcp->window = module__network__data__htons(64240);//8760);//64240);//0x1000);
  tcp->checksum = 0;
  tcp->urg_ptr = 0;

  module_kernel_memcpy(data, tcp->data, len);

  p->length = module__network__data__ntohs(ip->total_length)
    + sizeof(module__network__data__ethernet_header);

  module__network__data__packet_tcp_checksum(p);
  module__network__data__ip__checksum(p);
}
// -------------------------------------------------------------------------- //
void process_ip_tcp_packet(const module__network__data__packet * const p,
  module__network__ethernet_interface * const interface)
{
  (void)(p);//unused
  (void)(interface);//unused
//  const module__network__data__ip_header * const ip = ip_hdr(p);
//  const module__network__data__ip__tcp_header * const tcp = tcp_hdr(ip);

//  print_hex_bytes(p->buffer, p->length);
//  print_ip_tcp_header(tcp);

//  module_terminal_global_print_c_string("Processing IP_TCP packet\n");
//  module__network__test2();
}













// -------------------------------------------------------------------------- //
// Internet Protocol (IP) Packet
// -------------------------------------------------------------------------- //
void make_ip_packet(module__network__data__packet *p,
  const module__network__data__mac_address dest_mac,
  const uint32_t destination_ip, const void * const data, const size_t len,
  const module__network__ethernet_interface * const interface)
{
  module__network__data__ethernet_header *eth =
    module__network__data__packet_get_ethernet_header(p);
  eth->source_mac = interface->mac_address;

  eth->destination_mac = dest_mac;
  eth->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__ip_v4);

  module__network__data__ip_header *ip =
   module__network__data__packet_get_ip_header(p);
  ip->version = 4;
  ip->header_length = 5;
  ip->dscp = 0x10;
  ip->id = module__network__data__ntohs(45715);//);//1); //s->ip_id);
  ip->flags_frag = module__network__data__htons(0x4000);//0);//0x4000); // DNF
  ip->ttl = 64;
  ip->protocol = module__network__data__ethernet__ip__protocol_type__udp;
  ip->source_ip = module__network__data__htonl(interface->ip);
  ip->destination_ip = destination_ip;
  ip->total_length = module__network__data__htons(
    sizeof(module__network__data__ip_header) +
    sizeof(module__network__data__ip__tcp_header) + len);

  module_kernel_memcpy(data, ip->data, len);

  p->length = module__network__data__ntohs(ip->total_length)
    + sizeof(module__network__data__ethernet_header);

  module__network__data__ip__checksum(p);
}
// -------------------------------------------------------------------------- //
void process_ip_packet(const module__network__data__packet * const p,
  module__network__ethernet_interface * const interface)
{
  const module__network__data__ip_header * const ip =
    module__network__data__packet_get_ip_header_const(p);

  module_terminal_print_buffer_hex_bytes(p->buffer, p->length);
  module__network__data__packet_print_ip_header(ip);
  if (ip->destination_ip != module__network__data__htonl(interface->ip))
  {
    module_terminal_global_print_c_string("Got IP packet but not for my IP,"
      " ignoring.");
    return;
  }

  switch (ip->protocol)
  {
    case module__network__data__ethernet__ip__protocol_type__icmp:
      module_terminal_global_print_c_string("Handling IP_ICMP packet.\n");
//      echo_icmp(p);
      break;
    case module__network__data__ethernet__ip__protocol_type__tcp:
      module_terminal_global_print_c_string("Handling IP_TCP packet.\n");
      process_ip_tcp_packet(p, interface);
      break;
    case module__network__data__ethernet__ip__protocol_type__udp:
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
void module__network__process_ethernet_packet(
  const module__network__data__packet * const p,
  module__network__ethernet_interface * const interface)
{
//  print_hex_bytes(p->buffer, p->length);//rx_buff_size);
//  print_hex_bytes(rx_buffer, 64);//rx_buff_size);
  const module__network__data__ethernet_header * const eth =
    module__network__data__packet_get_ethernet_header_const(p);
  module__network__data__packet_print_ethernet_header(eth);
  const uint16_t eth_type = module__network__data__ntohs(eth->ethertype);
  if(eth_type == module__network__data__ethernet_header_type__arp)
  {
    process_arp_packet(p, interface);
  }
  else if(eth_type == module__network__data__ethernet_header_type__ip_v4)
  {
//    module_terminal_global_print_c_string("Got IP packet.\n");
    process_ip_packet(p, interface);
  }
  else
  {
//    module_terminal_global_print_c_string("Got UNKNOWN packet.\n");
  }
}







// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
void module__network__test()
{
  module__network__ethernet_interface * i =
    module__network__ethernet_interface__list; // first interface


  // DHCP discover - not really needed - but nice to have
  module__network__data__dhcp_config cfg;
  {
    cfg.ip = 0; // invalid value
    cfg.subnet_mask = 0; // invalid value
    cfg.gw = 0; // invalid value
    cfg.dns = 0; // invalid value
    cfg.dhcp_server_ip= 0; // invalid value
    cfg.lease_time_seconds = 0; // invalid value

    bool is_ok = module__network__service__dhcp__client__get_net_config(i,
      &cfg);
    if(!is_ok)
    {
      module_terminal_global_print_c_string("Problem while getting a"
        " DHCP configuration!\n");
      return;
    }

    module_terminal_global_print_c_string("Client DHCP configuration =\n{\n");
    module_terminal_global_print_c_string("  \"IP\"=\"");
    module__network__data__print_ipv4(cfg.ip);
    module_terminal_global_print_c_string("\",\n  \"subnet_mask\"=\"");
    module__network__data__print_ipv4(cfg.subnet_mask);
    module_terminal_global_print_c_string("\",\n  \"GW\"=\"");
    module__network__data__print_ipv4(cfg.gw);
    module_terminal_global_print_c_string("\",\n  \"DNS\"=\"");
    module__network__data__print_ipv4(cfg.dns);
    module_terminal_global_print_c_string("\",\n  \"DHCP_server_IP\"=\"");
    module__network__data__print_ipv4(cfg.dhcp_server_ip);
    module_terminal_global_print_c_string("\",\n  \"DHCP_server_MAC\"=\"");
    module__network__data__print_mac(&(cfg.dhcp_server_mac));
    module_terminal_global_print_c_string("\",\n  \"lease_seconds\"=\"");
    module_terminal_global_print_uint64(cfg.lease_time_seconds);
    module_terminal_global_print_c_string("s\"\n}\n");

    module_terminal_global_print_c_string("Remember that mac=");
    module__network__data__print_mac(&(cfg.dhcp_server_mac));
    module_terminal_global_print_c_string(" has IPv4=");
    module__network__data__print_ipv4(cfg.dhcp_server_ip);
    module_terminal_global_print_c_string(" .\n");
  }

  // Set config to interface
  i->ip = cfg.ip;
  i->subnet_mask = cfg.subnet_mask;
  i->gw = cfg.gw;
  i->dns = cfg.dns;

  // HTTP Client
//  module__network__service__http__client__request_response(i,
//    cfg.dhcp_server_ip, cfg.dhcp_server_mac, 1030);
  module__network__ip__tcp__test(i, cfg.dhcp_server_mac, cfg.dhcp_server_ip,
    1030);

  module_terminal_global_print_c_string("NET TEST END\n");
  return;


  // multicast listener report message v2 - not needed
//  {
//  module_terminal_global_print_c_string("Sending mlrmv2\n");
//  module__network__data__packet *p = new_pk_with_data(
//"\x33\x33\x00\x00\x00\x16\x52\x54\x00\x12\x13\x56\x86\xdd\x60\x00"
//"\x00\x00\x00\x24\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\xff\x02\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x16\x3a\x00\x05\x02\x00\x00\x01\x00\x8f\x00"
//"\x5c\x22\x00\x00\x00\x01\x04\x00\x00\x00\xff\x02\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x01\xff\x12\x13\x56"
//  , 90);
//  module_terminal_print_buffer_hex_bytes2((char*)(p->buffer), p->length);
//  module__driver__rtl8139__send_packet(p);
//  module__driver__rtl8139__send_packet(p);
//  free(p);
//  }











//  module_terminal_global_print_c_string("Sending ARP Query\n");
//  module__network__data__packet *request = new_pk();
//  arp_query(request, 167772674); // 10.0.2.2
//  print_hex_bytes(response->buffer, response->length);
//  const module__network__data__ip_header * const response_ip =
//    ip_hdr(response);
//  const module__network__data__ip__tcp_header * const response_tcp =
//    tcp_hdr(response_ip);
//  print_ip_tcp_header(response_tcp);
//  module__driver__rtl8139__send_packet(request);
//  free(request);
}
// -------------------------------------------------------------------------- //
