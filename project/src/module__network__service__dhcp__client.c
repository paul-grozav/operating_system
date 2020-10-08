// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module__network__service__dhcp__client.h"
#include "module_terminal.h"
#include "module_heap.h"
#include "module_kernel.h"
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
// -------------------------------------------------------------------------- //
void module__network__service__dhcp__client__get_net_config(
  module__network__ethernet_interface * const interface)
{
  {
    module_terminal_global_print_c_string("Sending DHCP discovery\n");
    const size_t len = 300;
    module__network__data__packet *p = module__network__data__packet__alloc();
//    p->length = 0; // populated later ?

    module__network__data__ethernet_header * eh =
      module__network__data__packet_get_ethernet_header(p);
    eh->destination_mac = module__network__data__mac_address__broadcast_mac;
    eh->source_mac = interface->mac_address;
    eh->ethertype = module__network__data__htons(
      module__network__data__ethernet_header_type__ip_v4);

    module__network__data__ip_header * iph =
      module__network__data__packet_get_ip_header(p);
    iph->version = 4;
    iph->header_length = 5;
    iph->dscp = 16;
//    iph->total_length = 0; // populated later ?
    iph->id = 0;
    iph->flags_frag = 0;
    iph->ttl = 128;
    iph->protocol = module__network__data__ethernet__ip__protocol_type__udp;
    iph->header_checksum = 0; // module__network__data__htons(0x3996); // 0; // populated later ?
    iph->source_ip = module__network__data__ip(0, 0, 0, 0);
    iph->destination_ip = module__network__data__ip(255, 255, 255, 255);

    module__network__data__ip__udp_header * udph =
      module__network__data__packet_get_ip_udp_header(iph);
    udph->source_port = module__network__data__htons(68);
    udph->destination_port = module__network__data__htons(67);
    udph->length = module__network__data__htons(
      sizeof(module__network__data__ip__udp_header) + len);
    udph->checksum = 0; // module__network__data__htons(0xd39b); // populated later ?

    module__network__data__ip__udp__bootp_header * bph =
      module__network__data__packet_get_ip_udp_bootp_header(udph);
    bph->op_code = 1; // 1 = Boot request
    bph->hardware_address_type = 1; // 1 = Ethernet
    bph->hardware_address_length = sizeof (module__network__data__mac_address);
    bph->gateway_hops = 0;
    bph->transaction_id = module__network__data__htonl(0x89cdf841); // ??? does it matter?
    bph->seconds_since_boot = 0; // just booted
    bph->flags = 0;
    bph->client_ip_address = module__network__data__ip(0, 0, 0, 0);
    bph->your_ip_address = module__network__data__ip(0, 0, 0, 0);
    bph->server_ip_address = module__network__data__ip(0, 0, 0, 0);
    bph->gateway_ip_address = module__network__data__ip(0, 0, 0, 0);
    // set mac, the rest of the 10 bytes will be free (0)
    module_kernel_memset(bph->client_hardware_address, 0, 16);
    *((module__network__data__mac_address *)(bph->client_hardware_address)) =
      interface->mac_address;
    module_kernel_memset(bph->server_host_name, 0, 64);
    module_kernel_memset(bph->boot_file_name, 0, 128);
    char * vd =
                              "\x63\x82\x53\x63\x35\x01\x01\x32\x04\x0a"\
      "\x00\x02\x0f\x0c\x06\x64\x65\x62\x69\x61\x6e\x37\x0d\x01\x1c\x02"\
      "\x03\x0f\x06\x77\x0c\x2c\x2f\x1a\x79\x2a\x3d\x13\xff\x00\x12\x13"\
      "\x56\x00\x01\x00\x01\x26\xd1\x38\xdb\x52\x54\x00\x12\x34\x56\xff"\
      "\x00\x00\x00\x00\x00\x00";
    module_kernel_memcpy(vd, bph->vendor_area, 64);


    iph->total_length = module__network__data__htons(
      sizeof(module__network__data__ip_header)
      + module__network__data__ntohs(udph->length)); // 0?
    p->length = module__network__data__ntohs(iph->total_length)
      + sizeof(module__network__data__ethernet_header);

    module__network__data__packet_udp_checksum(p);
    module__network__data__ip__checksum(p);


    module__network__data__packet * p2;
    p2 = module__network__data__packet__create_with_data(
"\xff\xff\xff\xff\xff\xff\x52\x54\x00\x12\x13\x56\x08\x00\x45\x10"
"\x01\x48\x00\x00\x00\x00\x80\x11\x39\x96\x00\x00\x00\x00\xff\xff"
"\xff\xff\x00\x44\x00\x43\x01\x34\xd3\x9b\x01\x01\x06\x00\x89\xcd"
"\xf8\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x52\x54\x00\x12\x13\x56\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x63\x82\x53\x63\x35\x01\x01\x32\x04\x0a"
"\x00\x02\x0f\x0c\x06\x64\x65\x62\x69\x61\x6e\x37\x0d\x01\x1c\x02"
"\x03\x0f\x06\x77\x0c\x2c\x2f\x1a\x79\x2a\x3d\x13\xff\x00\x12\x13"
"\x56\x00\x01\x00\x01\x26\xd1\x38\xdb\x52\x54\x00\x12\x34\x56\xff"
"\x00\x00\x00\x00\x00\x00"
  , 342);

    module_terminal_print_buffer_hex_bytes(p->buffer, 342);//(size_t)(p->length));

//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));

    module__network__ethernet_interface__send_packet(p, interface);
//    module__network__ethernet_interface__send_packet(p2, interface);
    free(p);
  }
//  module_terminal_global_print_c_string("Wait for packet on driver=");
//  module_terminal_global_print_hex_uint64((uint64_t)(i->driver));
//  module_terminal_global_print_c_string("\n");
  while(interface->ipq_index == 0){} // wait for packet
  const module__network__data__packet * const p =
    module__network__ethernet_interface__get_packet_from_incoming_queue(
    interface);
  module_terminal_print_buffer_hex_bytes(p->buffer, 10);//p->length);

  // DHCP request
//  {
//  module_terminal_global_print_c_string("Sending DHCP request\n");
//  module__network__data__packet *p = new_pk_with_data(
//"\xff\xff\xff\xff\xff\xff\x52\x54\x00\x12\x13\x56\x08\x00\x45\x10"
//"\x01\x48\x00\x00\x00\x00\x80\x11\x39\x96\x00\x00\x00\x00\xff\xff"
//"\xff\xff\x00\x44\x00\x43\x01\x34\xcb\x59\x01\x01\x06\x00\x89\xcd"
//"\xf8\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x52\x54\x00\x12\x13\x56\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
//"\x00\x00\x00\x00\x00\x00\x63\x82\x53\x63\x35\x01\x03\x36\x04\x0a"
//"\x00\x02\x02\x32\x04\x0a\x00\x02\x0f\x0c\x06\x64\x65\x62\x69\x61"
//"\x6e\x37\x0d\x01\x1c\x02\x03\x0f\x06\x77\x0c\x2c\x2f\x1a\x79\x2a"
//"\x3d\x13\xff\x00\x12\x13\x56\x00\x01\x00\x01\x26\xd1\x38\xdb\x52"
//"\x54\x00\x12\x34\x56\xff"
//  , 342);
//  module_terminal_print_buffer_hex_bytes2((char*)(p->buffer), p->length);
//  module__driver__rtl8139__send_packet(p);
//  free(p);
//  }
}
// -------------------------------------------------------------------------- //
