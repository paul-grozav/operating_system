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
void module__network__service__dhcp__client__step1__send_dhcp_discover_packet(
  module__network__ethernet_interface * const interface)
{
  module_terminal_global_print_c_string("Sending DHCP discover packet\n");
  module__network__data__packet *p = module__network__data__packet__alloc();
  p->length = sizeof(module__network__data__ethernet_header)
    + sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__udp_header)
    + sizeof(module__network__data__ip__udp__bootp_header);

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
  iph->total_length = module__network__data__htons(
    sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__udp_header)
    + sizeof(module__network__data__ip__udp__bootp_header));
  iph->id = 0;
  iph->flags_frag = 0;
  iph->ttl = 128;
  iph->protocol = module__network__data__ethernet__ip__protocol_type__udp;
  iph->header_checksum = 0; // populated later at the end, after data
  iph->source_ip = module__network__data__ip(0, 0, 0, 0);
  iph->destination_ip = module__network__data__ip(255, 255, 255, 255);

  module__network__data__ip__udp_header * udph =
    module__network__data__packet_get_ip_udp_header(iph);
  udph->source_port = module__network__data__htons(68);
  udph->destination_port = module__network__data__htons(67);
  udph->length = module__network__data__htons(
    sizeof(module__network__data__ip__udp_header)
    + sizeof(module__network__data__ip__udp__bootp_header));
  udph->checksum = 0; // populated at the end, after data

  module__network__data__ip__udp__bootp_header * bph =
    module__network__data__packet_get_ip_udp_bootp_header(udph);
  bph->op_code = 1; // 1 = Boot request
  bph->hardware_address_type = 1; // 1 = Ethernet
  bph->hardware_address_length = sizeof (module__network__data__mac_address);
  bph->gateway_hops = 0;
  bph->transaction_id = module__network__data__htonl(1);//0x89cdf841);//random
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
  {
    module_kernel_memset(bph->vendor_area, 0, 64); // 64 = vendor area size

    size_t i = 0;
    // DHCP magic cookie - an identifier
    *(uint32_t *)(void*)(bph->vendor_area + i) =
      module__network__data__htonl(0x63825363); // important to keep it.
    i += sizeof(uint32_t);

    // Option 53 - DHCP message type
    *(uint8_t *)(bph->vendor_area + i++) = 53;
    *(uint8_t *)(bph->vendor_area + i++) = 1; // length = 1
    *(uint8_t *)(bph->vendor_area + i++) = 1; // value 1 = DHCP discover

    // Option 50 - requested IP address - last known ip address.
    // is not mandatory that it will give you this address
    *(uint8_t *)(bph->vendor_area + i++) = 50;
    *(uint8_t *)(bph->vendor_area + i++) = 4; // length = 4
    *(uint32_t *)(void*)(bph->vendor_area + i) =
      module__network__data__ip(192, 168, 0, 13); // value 1 = wanted IP addr
    i += sizeof(uint32_t);

    // Option 12 - client host name
    *(uint8_t *)(bph->vendor_area + i++) = 12;
    *(uint8_t *)(bph->vendor_area + i++) = 6; // length = 6
    module_kernel_memcpy("TediOS", (bph->vendor_area + i), 6);
    i += 6;

    // Option 55 - Parameter request list
    *(uint8_t *)(bph->vendor_area + i++) = 55;
    *(uint8_t *)(bph->vendor_area + i++) = 13; // length = 13
    // List items
    *(uint8_t *)(bph->vendor_area + i++) = 1; // subnet mask
    *(uint8_t *)(bph->vendor_area + i++) = 28; // broadcast address
    *(uint8_t *)(bph->vendor_area + i++) = 2; // time offset
    *(uint8_t *)(bph->vendor_area + i++) = 3; // router
    *(uint8_t *)(bph->vendor_area + i++) = 15; // domain name
    *(uint8_t *)(bph->vendor_area + i++) = 6; // domain name server
    *(uint8_t *)(bph->vendor_area + i++) = 119; // domain search
    *(uint8_t *)(bph->vendor_area + i++) = 12; // host name
    *(uint8_t *)(bph->vendor_area + i++) = 44; // NetBIOS over TCP/IP name srv
    *(uint8_t *)(bph->vendor_area + i++) = 47; // NetBIOS over TCP/IP scope
    *(uint8_t *)(bph->vendor_area + i++) = 26; // interface mtu
    *(uint8_t *)(bph->vendor_area + i++) = 121; // classless static route
    *(uint8_t *)(bph->vendor_area + i++) = 42; // NetTimeProto servers

    // Option 61 - client identifier
    *(uint8_t *)(bph->vendor_area + i++) = 61;
    *(uint8_t *)(bph->vendor_area + i++) = 19; // length = 19
    *(uint8_t *)(bph->vendor_area + i++) = 255; // what is this ? padding ??
    *(uint32_t *)(void*)(bph->vendor_area + i) =
      module__network__data__htonl(1);//0x00121356); // IAID
    i += sizeof(uint32_t);
    *(uint16_t *)(void*)(bph->vendor_area + i) =
      module__network__data__htons(1); // DUID type= link layer address + time
    i += sizeof(uint16_t);
    *(uint16_t *)(void*)(bph->vendor_area + i) =
      module__network__data__htons(1); // Hardware type = ethernet
    i += sizeof(uint16_t);
    *(uint32_t *)(void*)(bph->vendor_area + i) =
      module__network__data__htonl(1);//0x26d138db); // Time
    i += sizeof(uint32_t);
    *((module__network__data__mac_address *)(bph->vendor_area + i)) =
      interface->mac_address;
    i += sizeof(module__network__data__mac_address);

    // Option 255 - END of options list - no length & value
    *(uint8_t *)(bph->vendor_area + i++) = 255;
  }
  // populate checksums for udp and ip headers
  module__network__data__packet_udp_checksum(p);
  module__network__data__ip__checksum(p);

//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));

  module__network__ethernet_interface__send_packet(p, interface);
  free(p);
}
// -------------------------------------------------------------------------- //
void module__network__service__dhcp__client__step2__receive_dhcp_offer_packet(
  module__network__ethernet_interface * const interface)
{
  module_terminal_global_print_c_string("Waiting for DHCP offer packet on"
    " driver=");
  module_terminal_global_print_hex_uint64((uint32_t)(interface->driver));
  module_terminal_global_print_c_string(" ...\n");
  while(interface->ipq_index == 0){} // wait for a packet
  // got a packet ... maybe notthe right one ...
  const module__network__data__packet * const p =
    module__network__ethernet_interface__get_packet_from_incoming_queue(
    interface);
//  module_terminal_print_buffer_hex_bytes(p->buffer, 10);//p->length);

  const module__network__data__ethernet_header * const eh =
    module__network__data__packet_get_ethernet_header_const(p);
  if(module_kernel_memcmp(
    &(eh->destination_mac),
    &module__network__data__mac_address__broadcast_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
    module_terminal_global_print_c_string("Not for broadcast mac.\n");
    return;
  }

  if(module__network__data__ntohs(eh->ethertype)
    != module__network__data__ethernet_header_type__ip_v4)
  {
    module_terminal_global_print_c_string("Not IPv4.\n");
    return;
  }


  const module__network__data__ip_header * const iph =
    module__network__data__packet_get_ip_header_const(p);

  module_terminal_global_print_c_string("Remember that mac=");
  module__network__data__print_mac(&(eh->source_mac));
  module_terminal_global_print_c_string(" has IPv4=");
  module__network__data__print_ipv4(iph->source_ip);
  module_terminal_global_print_c_string(" .\n");

  if(iph->version != 4)
  {
    module_terminal_global_print_c_string("Not IPv4 IP header.\n");
    return;
  }

  if(iph->destination_ip != module__network__data__ip(255, 255, 255, 255))
  {
    module_terminal_global_print_c_string("Not sent to broadcast IPv4.\n");
    return;
  }


  const module__network__data__ip__udp_header * const udph =
    module__network__data__packet_get_ip_udp_header_const(iph);

  if(module__network__data__ntohs(udph->source_port) != 67)
  {
    module_terminal_global_print_c_string("DHCP offer should come from port=67"
      ".\n");
    return;
  }

  if(module__network__data__ntohs(udph->destination_port) != 68)
  {
    module_terminal_global_print_c_string("DHCP offer should go to port=67.\n");
    return;
  }


  const module__network__data__ip__udp__bootp_header * const bph =
    module__network__data__packet_get_ip_udp_bootp_header_const(udph);

  if(bph->op_code != 2)
  {
    module_terminal_global_print_c_string("DHCP not boot reply.\n");
    return;
  }

  if(bph->hardware_address_type != 1)
  {
    module_terminal_global_print_c_string("DHCP to addr not ethernet.\n");
    return;
  }

  if(bph->hardware_address_length != sizeof(module__network__data__mac_address))
  {
    module_terminal_global_print_c_string("DHCP to addr len not mac.\n");
    return;
  }

  if(module__network__data__ntohl(bph->transaction_id) != 1)
  {
    // should be the same id as given in discover
    module_terminal_global_print_c_string("DHCP transaction not OK.\n");
    return;
  }

  if(module__network__data__ntohl(
    *(const uint32_t *)(const void*)(bph->vendor_area + 0)) != 0x63825363)
  {
    // DHCP magic cookie check failed
    module_terminal_global_print_c_string("DHCP cookie check failed.\n");
    return;
  }

  if(module_kernel_memcmp(
    &(bph->client_hardware_address),
    &(interface->mac_address),
    sizeof(module__network__data__mac_address))
    != 0)
  {
    module_terminal_global_print_c_string("Not for my mac.\n");
    return;
  }

  typedef struct {
    uint32_t ip;
    uint32_t subnet_mask;
    uint32_t gw;
    uint32_t dns;
    uint32_t lease_time_seconds;
  } dhcp_config;
  dhcp_config cfg;
  cfg.ip = bph->your_ip_address;
  cfg.subnet_mask = 0; // invalid value
  cfg.gw = bph->server_ip_address;
  cfg.dns = 0; // invalid value
  cfg.lease_time_seconds = 0; // invalid value

  // Process BootP options from vendor area
  {
    size_t i = 0;
    i += sizeof(uint32_t); // skip DHCP magic cookie - an identifier
    const uint8_t * option_number = NULL;
    const uint8_t * option_value_length = NULL;
    while(i < 64) // 64 = vendor area size
    {
      option_number = (bph->vendor_area + i++);
      option_value_length = (bph->vendor_area + i++);
//      module_terminal_global_print_c_string("OPTION=");
//      module_terminal_global_print_uint64(*option_number);
//      module_terminal_global_print_c_string("\n");
//      module_terminal_global_print_c_string("LEN=");
//      module_terminal_global_print_uint64(*option_value_length);
//      module_terminal_global_print_c_string("\n");
//      module_terminal_global_print_c_string("VAL=");
//      module_terminal_global_print_uint64(*(bph->vendor_area + i));
//      module_terminal_global_print_c_string("\n");
//      module_terminal_global_print_c_string("\n");
      if(*option_number == 1) // Subnet mask
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("subnet not on 4 bytes.\n");
          return;
        }
        cfg.subnet_mask =
          *(const uint32_t *)(const void*)(bph->vendor_area + i);
      }
      else if(*option_number == 3) // Router
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("router not on 4 bytes.\n");
          return;
        }
        cfg.gw = *(const uint32_t *)(const void*)(bph->vendor_area + i);
      }
      else if(*option_number == 6) // Domain Name Server (DNS)
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("DNS not on 4 bytes.\n");
          return;
        }
        cfg.dns = *(const uint32_t *)(const void*)(bph->vendor_area + i);
      }
      else if(*option_number == 51) // IP Address lease time
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("DHCP Lease not on 4 bytes.\n");
          return;
        }
        cfg.lease_time_seconds = module__network__data__ntohl(
          *(const uint32_t *)(const void*)(bph->vendor_area + i));
      }
      else if(*option_number == 53) // DHCP message type
      {
        if( (*option_value_length != 1 )
          || (*(const uint8_t *)(const void*)(bph->vendor_area + i) != 2))
        {
          // 2 = offer
          module_terminal_global_print_c_string("Not DHCP offer option.\n");
          return;
        }
      }
      else if(*option_number == 255) // END of options list
      {
        break;
      }
      else
      {
        // ignore this option - not interesting
      }
      i += *option_value_length; // skip value
    }
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
  module_terminal_global_print_c_string("\",\n  \"lease_seconds\"=\"");
  module_terminal_global_print_uint64(cfg.lease_time_seconds);
  module_terminal_global_print_c_string("s\"\n}\n");

//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));
}
// -------------------------------------------------------------------------- //
void module__network__service__dhcp__client__get_net_config(
  module__network__ethernet_interface * const interface)
{
  module__network__service__dhcp__client__step1__send_dhcp_discover_packet(
    interface);
  module__network__service__dhcp__client__step2__receive_dhcp_offer_packet(
    interface);

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
