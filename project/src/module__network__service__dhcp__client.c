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
    *(uint32_t *)(void*)(bph->vendor_area + i) = // any junk here ...
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
bool module__network__service__dhcp__client__step2__receive_dhcp_offer_packet(
  module__network__ethernet_interface * const interface,
  module__network__data__dhcp_config * const cfg)
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
    return false;
  }

  if(module__network__data__ntohs(eh->ethertype)
    != module__network__data__ethernet_header_type__ip_v4)
  {
    module_terminal_global_print_c_string("Not IPv4.\n");
    return false;
  }


  const module__network__data__ip_header * const iph =
    module__network__data__packet_get_ip_header_const(p);

  if(iph->version != 4)
  {
    module_terminal_global_print_c_string("Not IPv4 IP header.\n");
    return false;
  }

  if(iph->destination_ip != module__network__data__ip(255, 255, 255, 255))
  {
    module_terminal_global_print_c_string("Not sent to broadcast IPv4.\n");
    return false;
  }


  const module__network__data__ip__udp_header * const udph =
    module__network__data__packet_get_ip_udp_header_const(iph);

  if(module__network__data__ntohs(udph->source_port) != 67)
  {
    module_terminal_global_print_c_string("DHCP offer should come from port=67"
      ".\n");
    return false;
  }

  if(module__network__data__ntohs(udph->destination_port) != 68)
  {
    module_terminal_global_print_c_string("DHCP offer should go to port=67.\n");
    return false;
  }


  const module__network__data__ip__udp__bootp_header * const bph =
    module__network__data__packet_get_ip_udp_bootp_header_const(udph);

  if(bph->op_code != 2)
  {
    module_terminal_global_print_c_string("DHCP not boot reply.\n");
    return false;
  }

  if(bph->hardware_address_type != 1)
  {
    module_terminal_global_print_c_string("DHCP to addr not ethernet.\n");
    return false;
  }

  if(bph->hardware_address_length != sizeof(module__network__data__mac_address))
  {
    module_terminal_global_print_c_string("DHCP to addr len not mac.\n");
    return false;
  }

  if(module__network__data__ntohl(bph->transaction_id) != 1)
  {
    // should be the same id as given in discover
    module_terminal_global_print_c_string("DHCP transaction not OK.\n");
    return false;
  }

  if(module__network__data__ntohl(
    *(const uint32_t *)(const void*)(bph->vendor_area + 0)) != 0x63825363)
  {
    // DHCP magic cookie check failed
    module_terminal_global_print_c_string("DHCP cookie check failed.\n");
    return false;
  }

  if(module_kernel_memcmp(
    &(bph->client_hardware_address),
    &(interface->mac_address),
    sizeof(module__network__data__mac_address))
    != 0)
  {
    module_terminal_global_print_c_string("Not for my mac.\n");
    return false;
  }

  cfg->ip = bph->your_ip_address;
  cfg->gw = bph->server_ip_address;

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
          return false;
        }
        cfg->subnet_mask =
          *(const uint32_t *)(const void*)(bph->vendor_area + i);
      }
      else if(*option_number == 3) // Router
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("router not on 4 bytes.\n");
          return false;
        }
        cfg->gw = *(const uint32_t *)(const void*)(bph->vendor_area + i);
      }
      else if(*option_number == 6) // Domain Name Server (DNS)
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("DNS not on 4 bytes.\n");
          return false;
        }
        cfg->dns = *(const uint32_t *)(const void*)(bph->vendor_area + i);
      }
      else if(*option_number == 51) // IP Address lease time
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("DHCP Lease not on 4 bytes.\n");
          return false;
        }
        cfg->lease_time_seconds = module__network__data__ntohl(
          *(const uint32_t *)(const void*)(bph->vendor_area + i));
      }
      else if(*option_number == 53) // DHCP message type
      {
        if( (*option_value_length != 1 )
          || (*(const uint8_t *)(const void*)(bph->vendor_area + i) != 2))
        {
          // 2 = offer
          module_terminal_global_print_c_string("Not DHCP offer option.\n");
          return false;
        }
      }
      else if(*option_number == 54) // DHCP server identifier
      {
        if(*option_value_length != 4 )
        {
          module_terminal_global_print_c_string("DHCP srv id len != 4 .\n");
          return false;
        }
        cfg->dhcp_server_ip =
          *(const uint32_t *)(const void*)(bph->vendor_area + i);
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
  module_kernel_memcpy(&(eh->source_mac), &(cfg->dhcp_server_mac),
    sizeof(module__network__data__mac_address));
//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));
  return true;
}
// -------------------------------------------------------------------------- //
void module__network__service__dhcp__client__step3__send_dhcp_request_packet(
  module__network__ethernet_interface * const interface,
  const module__network__data__dhcp_config * const cfg)
{
  module_terminal_global_print_c_string("Sending DHCP request packet\n");
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
    *(uint8_t *)(bph->vendor_area + i++) = 3; // value 3 = DHCP request

    // Option 54 - DHCP server identifier
    *(uint8_t *)(bph->vendor_area + i++) = 54;
    *(uint8_t *)(bph->vendor_area + i++) = 4; // length = 4
    *(uint32_t *)(void*)(bph->vendor_area + i) = cfg->dhcp_server_ip;// ip of
    // the dhcp server that proposed that IP to me.
    i += sizeof(uint32_t);

    // Option 50 - requested IP address - last known ip address.
    // is not mandatory that it will give you this address
    *(uint8_t *)(bph->vendor_area + i++) = 50;
    *(uint8_t *)(bph->vendor_area + i++) = 4; // length = 4
    *(uint32_t *)(void*)(bph->vendor_area + i) = cfg->ip; // value = wanted IP
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
bool
  module__network__service__dhcp__client__step4__receive_dhcp_acknowledge_packet
  (module__network__ethernet_interface * const interface,
  const module__network__data__dhcp_config * const cfg)
{
  module_terminal_global_print_c_string("Waiting for DHCP acknowledge packet on"
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
    return false;
  }

  if(module__network__data__ntohs(eh->ethertype)
    != module__network__data__ethernet_header_type__ip_v4)
  {
    module_terminal_global_print_c_string("Not IPv4.\n");
    return false;
  }


  const module__network__data__ip_header * const iph =
    module__network__data__packet_get_ip_header_const(p);

  if(iph->version != 4)
  {
    module_terminal_global_print_c_string("Not IPv4 IP header.\n");
    return false;
  }

  if(iph->destination_ip != module__network__data__ip(255, 255, 255, 255))
  {
    module_terminal_global_print_c_string("Not sent to broadcast IPv4.\n");
    return false;
  }


  const module__network__data__ip__udp_header * const udph =
    module__network__data__packet_get_ip_udp_header_const(iph);

  if(module__network__data__ntohs(udph->source_port) != 67)
  {
    module_terminal_global_print_c_string("DHCP offer should come from port=67"
      ".\n");
    return false;
  }

  if(module__network__data__ntohs(udph->destination_port) != 68)
  {
    module_terminal_global_print_c_string("DHCP offer should go to port=67.\n");
    return false;
  }


  const module__network__data__ip__udp__bootp_header * const bph =
    module__network__data__packet_get_ip_udp_bootp_header_const(udph);

  if(bph->op_code != 2)
  {
    module_terminal_global_print_c_string("DHCP not boot reply.\n");
    return false;
  }

  if(bph->hardware_address_type != 1)
  {
    module_terminal_global_print_c_string("DHCP to addr not ethernet.\n");
    return false;
  }

  if(bph->hardware_address_length != sizeof(module__network__data__mac_address))
  {
    module_terminal_global_print_c_string("DHCP to addr len not mac.\n");
    return false;
  }

  if(module__network__data__ntohl(bph->transaction_id) != 1)
  {
    // should be the same id as given in discover
    module_terminal_global_print_c_string("DHCP transaction not OK.\n");
    return false;
  }

  if(bph->your_ip_address != cfg->ip)
  {
    // should be the same ip as given in offer and request
    module_terminal_global_print_c_string("DHCP ACK IP not OK.\n");
    return false;
  }

  if(module__network__data__ntohl(
    *(const uint32_t *)(const void*)(bph->vendor_area + 0)) != 0x63825363)
  {
    // DHCP magic cookie check failed
    module_terminal_global_print_c_string("DHCP cookie check failed.\n");
    return false;
  }

  if(module_kernel_memcmp(
    &(bph->client_hardware_address),
    &(interface->mac_address),
    sizeof(module__network__data__mac_address))
    != 0)
  {
    module_terminal_global_print_c_string("Not for my mac.\n");
    return false;
  }

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

//      if(*option_number == 1) // Subnet mask
//      {
//        if(*option_value_length != 4 )
//        {
//          module_terminal_global_print_c_string("subnet not on 4 bytes.\n");
//          return;
//        }
//      }
//      else if(*option_number == 3) // Router
//      {
//        if(*option_value_length != 4 )
//        {
//          module_terminal_global_print_c_string("router not on 4 bytes.\n");
//          return;
//        }
//      }
//      else if(*option_number == 6) // Domain Name Server (DNS)
//      {
//        if(*option_value_length != 4 )
//        {
//          module_terminal_global_print_c_string("DNS not on 4 bytes.\n");
//          return;
//        }
//      }
//      else if(*option_number == 51) // IP Address lease time
//      {
//        if(*option_value_length != 4 )
//        {
//          module_terminal_global_print_c_string("DHCP Lease"
//            " not on 4 bytes.\n");
//          return;
//        }
//      }
//      else
      if(*option_number == 53) // DHCP message type
      {
        if( (*option_value_length != 1 )
          || (*(const uint8_t *)(const void*)(bph->vendor_area + i) != 5))
        {
          // 5 = acknowledge
          module_terminal_global_print_c_string("Not DHCP offer option.\n");
          return false;
        }
      }
//      else if(*option_number == 54) // DHCP server identifier
//      {
//        if(*option_value_length != 4 )
//        {
//          module_terminal_global_print_c_string("DHCP srv id len != 4 .\n");
//          return;
//        }
//      }
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

//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));
  return true;
}
// -------------------------------------------------------------------------- //
void module__network__service__dhcp__client__get_net_config(
  module__network__ethernet_interface * const interface)
{
  module__network__data__dhcp_config cfg;
  cfg.ip = 0; // invalid value
  cfg.subnet_mask = 0; // invalid value
  cfg.gw = 0; // invalid value
  cfg.dns = 0; // invalid value
  cfg.dhcp_server_ip= 0; // invalid value
  cfg.lease_time_seconds = 0; // invalid value

  bool is_ok = false;
  module__network__service__dhcp__client__step1__send_dhcp_discover_packet(
    interface);
  is_ok =
    module__network__service__dhcp__client__step2__receive_dhcp_offer_packet(
    interface, &cfg);

  if(!is_ok)
  {
    module_terminal_global_print_c_string("Problem while asking for a"
      " DHCP configuration!\n");
  }

  module__network__service__dhcp__client__step3__send_dhcp_request_packet(
    interface, &cfg);
  is_ok =
  module__network__service__dhcp__client__step4__receive_dhcp_acknowledge_packet
    (interface, &cfg);

  if(!is_ok)
  {
    module_terminal_global_print_c_string("Problem while requesting a"
      " DHCP configuration!\n");
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
// -------------------------------------------------------------------------- //
