// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module__network__service__http__client.h"
#include "module_terminal.h"
#include "module_heap.h"
#include "module_kernel.h"
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
// -------------------------------------------------------------------------- //
bool module__network__service__http__client__tcp_connection_init(
  module__network__ethernet_interface * const interface,
  const uint32_t server_ip, const module__network__data__mac_address server_mac,
  const uint16_t server_port, const uint16_t source_port)
{
  const size_t len = 20;
  const uint8_t flags = module__network__data__ip__tcp_flag__syn;
  module_terminal_global_print_c_string("TCP Conn init\n");
  module__network__data__packet *p = module__network__data__packet__alloc();
  p->length = sizeof(module__network__data__ethernet_header)
    + sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len
  ;

  module__network__data__ethernet_header * eh =
    module__network__data__packet_get_ethernet_header(p);
  eh->destination_mac = server_mac;
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
    + sizeof(module__network__data__ip__tcp_header)
    + len);
  iph->id = 0;
  iph->flags_frag = 0;
  iph->ttl = 128;
  iph->protocol = module__network__data__ethernet__ip__protocol_type__tcp;
  iph->header_checksum = 0; // populated later at the end, after data
  iph->source_ip = interface->ip;
  iph->destination_ip = server_ip;

  module__network__data__ip__tcp_header * tcph =
    module__network__data__packet_get_ip_tcp_header(iph);

  tcph->source_port = module__network__data__htons(source_port); // random
  tcph->destination_port = module__network__data__htons(server_port);
  tcph->seq = 0;//module__network__data__htonl(0xdb7fa4f6); // ???
  if (flags & module__network__data__ip__tcp_flag__ack)
  {
    tcph->ack = module__network__data__htonl(0);//1); //s->recv_seq);
  }
  else
  {
    tcph->ack = 0;
  }
  tcph->f_ns = 0; // 1 bit
  tcph->_reserved = 0; // 3 bits
  tcph->offset = 10;//6;//10/2;//5; // 5=no_opt & 10=opt // 4 bits
  tcph->f_fin = ((flags & module__network__data__ip__tcp_flag__fin) > 0);
  tcph->f_syn = ((flags & module__network__data__ip__tcp_flag__syn) > 0);
  tcph->f_rst = ((flags & module__network__data__ip__tcp_flag__rst) > 0);
  tcph->f_psh = ((flags & module__network__data__ip__tcp_flag__psh) > 0);
  tcph->f_ack = ((flags & module__network__data__ip__tcp_flag__ack) > 0);
  tcph->f_urg = ((flags & module__network__data__ip__tcp_flag__urg) > 0);
  tcph->f_ece = 0;
  tcph->f_cwr = 0;
  tcph->window = module__network__data__htons(64240);
  tcph->checksum = 0;
  tcph->urg_ptr = 0;

  // TCP Options (length of all data - options ... is ... 20 bytes)
  {
    size_t i = 0;
    // Note: Option length contains the first byte (option ID) and the byte that
    // contains the length itself. So a length of 4 means that 1 byte is used
    // for the option id, 1 for the length, and you still have 2 for the value.
    // While a length of 2 means that there is no value, it's just a flag

    *(uint8_t *)(tcph->data + i++) = 2; // option 2 = maximum segment size (mss)
    *(uint8_t *)(tcph->data + i++) = 4; // length = 4
    *(uint16_t *)(void*)(tcph->data + i) = module__network__data__htons(1460);
    i += sizeof(uint16_t);

    *(uint8_t *)(tcph->data + i++) = 4; // option 4 = SACK permitted
    *(uint8_t *)(tcph->data + i++) = 2; // length = 2

    *(uint8_t *)(tcph->data + i++) = 8; // option 8 = time stamp option
    *(uint8_t *)(tcph->data + i++) = 10; // length = 4
    *(uint32_t *)(void*)(tcph->data + i) =
      module__network__data__htonl(3635825962); // timestamp value
    i += sizeof(uint32_t);
    *(uint32_t *)(void*)(tcph->data + i) = module__network__data__htonl(0);
    i += sizeof(uint32_t); // timestamp echo reply

    *(uint8_t *)(tcph->data + i++) = 1; // option 1 = No Operation
    // no length, and no value

    *(uint8_t *)(tcph->data + i++) = 3; // option 3 = Window scale
    *(uint8_t *)(tcph->data + i++) = 3; // length = 3
    *(uint8_t *)(tcph->data + i++) = 7; // value = shift count. multiplier = 128
  }

//  p->length = module__network__data__ntohs(iph->total_length)
//    + sizeof(module__network__data__ethernet_header);

  module__network__data__packet_tcp_checksum(p);
  module__network__data__ip__checksum(p);

  //  module_terminal_print_buffer_hex_bytes2((char*)(p->buffer), p->length);
  module__network__ethernet_interface__send_packet(p, interface);
  free(p);
  return true;
}
// -------------------------------------------------------------------------- //
bool module__network__service__http__client__request_response(
  module__network__ethernet_interface * const interface,
  const uint32_t server_ip, const module__network__data__mac_address server_mac,
  const uint16_t server_port)
{
  const uint16_t source_port = 54321;
  // TCP HTTP Request-Response
  module__network__service__http__client__tcp_connection_init(interface,
    server_ip, server_mac, server_port, source_port);


////  return;
//  module_terminal_global_print_c_string("Waiting for syn-ack\n");
//  for(size_t i=1; i<99999999; i++)
//  {
//    // waiting
////    module_terminal_global_print_c_string(".");
//  }
//  {
//  module_terminal_global_print_c_string("TCP Conn ack\n");
//  module__network__data__packet *p = new_pk_with_data(
//"\x52\x55\x0a\x00\x02\x02\x52\x54\x00\x12\x13\x56\x08\x00\x45\x00"
//"\x00\x28\x5c\x84\x40\x00\x40\x06\xc6\x3b\x0a\x00\x02\x0f\x0a\x00"
//"\x02\x02\xa3\x84\x04\x06\xdb\x7f\xa4\xf7\x00\x00\xfa\x02\x50\x10"
//"\xfa\xf0\x7a\xce\x00\x00"
//  , 54);
////  module_terminal_print_buffer_hex_bytes2((char*)(p->buffer), p->length);
//  module__network__ethernet_interface__send_packet(p, i);
//  free(p);
//  }
//  {
//  module_terminal_global_print_c_string("HTTP req\n");
//  module__network__data__packet *p = new_pk_with_data(
//"\x52\x55\x0a\x00\x02\x02\x52\x54\x00\x12\x13\x56\x08\x00\x45\x00"
//"\x00\xb4\x5c\x85\x40\x00\x40\x06\xc5\xae\x0a\x00\x02\x0f\x0a\x00"
//"\x02\x02\xa3\x84\x04\x06\xdb\x7f\xa4\xf7\x00\x00\xfa\x02\x50\x18"
//"\xfa\xf0\xa9\xc9\x00\x00\x47\x45\x54\x20\x2f\x20\x48\x54\x54\x50"
//"\x2f\x31\x2e\x31\x0d\x0a\x55\x73\x65\x72\x2d\x41\x67\x65\x6e\x74"
//"\x3a\x20\x57\x67\x65\x74\x2f\x31\x2e\x32\x30\x2e\x31\x20\x28\x6c"
//"\x69\x6e\x75\x78\x2d\x67\x6e\x75\x29\x0d\x0a\x41\x63\x63\x65\x70"
//"\x74\x3a\x20\x2a\x2f\x2a\x0d\x0a\x41\x63\x63\x65\x70\x74\x2d\x45"
//"\x6e\x63\x6f\x64\x69\x6e\x67\x3a\x20\x69\x64\x65\x6e\x74\x69\x74"
//"\x79\x0d\x0a\x48\x6f\x73\x74\x3a\x20\x31\x30\x2e\x30\x2e\x32\x2e"
//"\x32\x3a\x31\x30\x33\x30\x0d\x0a\x43\x6f\x6e\x6e\x65\x63\x74\x69"
//"\x6f\x6e\x3a\x20\x4b\x65\x65\x70\x2d\x41\x6c\x69\x76\x65\x0d\x0a"
//"\x0d\x0a"
//  , 194);
////  module_terminal_print_buffer_hex_bytes2((char*)(p->buffer), p->length);
//  module__network__ethernet_interface__send_packet(p, i);
//  free(p);
//  }



//  return;
//  module_terminal_global_print_c_string("Sending IP_TCP packet\n");
//  module__network__data__packet *response = new_pk();
//  uint32_t dst_ip = 0;
////  dst_ip = 3627734734; // = 216.58.214.206 = google.com
////  dst_ip = 3232286790; // 192.168.200.70 = host
//  dst_ip = 167772674; // 10.0.2.2 = GW
//  dst_ip = module__network__data__htonl(dst_ip);
//  uint16_t dst_port = module__network__data__htons(1030); // web server
//  make_ip_tcp_packet(response, dst_ip, dst_port,
//    module__network__data__htons(9876),
//    (module__network__data__ip__tcp_flag__syn),
//    //"", 0
//    "\x02\x04\x05\xb4\x04\x02\x08\x0a\xb3\x7f\x12\xcc\x0\x0\x0\x0\x01\x03\x03\x07", 20
//    //"\x02\x04\x05\xb4", 4
//    , i);
//  module_terminal_print_buffer_hex_bytes(response->buffer, response->length);
//  const module__network__data__ip_header * const response_ip = ip_hdr(response);
//  const module__network__data__ip__tcp_header * const response_tcp =
//    tcp_hdr(response_ip);
//  print_ip_tcp_header(response_tcp);
//  module__network__ethernet_interface__send_packet(response, i);
//  free(response);
  return true;
}
// -------------------------------------------------------------------------- //