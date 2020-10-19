// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// https://packetlife.net/blog/2010/jun/7/understanding-tcp-sequence-
// acknowledgment-numbers/
// -------------------------------------------------------------------------- //
#include "module_terminal.h"
#include "module_kernel.h"
#include "module_heap.h"
#include "module__network__ip__tcp.h"
// -------------------------------------------------------------------------- //
void module__network__ip__tcp__session_create(
  module__network__ip__tcp__session * const session,
  const module__network__data__mac_address source_mac,
  const uint32_t source_ip,
  const uint16_t source_port,
  const module__network__data__mac_address destination_mac,
  const uint32_t destination_ip,
  const uint16_t destination_port
  )
{
  session->source_mac = source_mac;
  session->source_ip = source_ip;
  session->source_port = source_port;

  session->destination_mac = destination_mac;
  session->destination_ip = destination_ip;
  session->destination_port = destination_port;

  session->connection_state =
    module__network__ip__tcp__session__connection_state__not_initiated;
  session->seq = 0;
  session->ack = 0;
}
// -------------------------------------------------------------------------- //
void module__network__ip__tcp__connect__init(
  module__network__ethernet_interface * const interface,
  module__network__ip__tcp__session * const session)
{
  // Send SYN packet
  const size_t len = 20;
  const uint8_t flags = module__network__data__ip__tcp_flag__syn;
  module__network__data__packet *p = module__network__data__packet__alloc();
  p->length = sizeof(module__network__data__ethernet_header)
    + sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len
  ;

  module__network__data__ethernet_header * eh =
    module__network__data__packet_get_ethernet_header(p);
  eh->destination_mac = session->destination_mac;
  eh->source_mac = session->source_mac;
  eh->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__ip_v4);

  module__network__data__ip_header * iph =
    module__network__data__packet_get_ip_header(p);
  iph->version = 4;
  iph->header_length = 5;
  iph->dscp = 0;
  iph->total_length = module__network__data__htons(
    sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len);
  iph->id = module__network__data__htons(0);
  iph->flags_frag = module__network__data__htons(1 << 14); // Don't Fragment bit
  iph->ttl = 64;
  iph->protocol = module__network__data__ethernet__ip__protocol_type__tcp;
  iph->header_checksum = 0; // populated later at the end, after data
  iph->source_ip = session->source_ip;
  iph->destination_ip = session->destination_ip;

  module__network__data__ip__tcp_header * tcph =
    module__network__data__packet_get_ip_tcp_header(iph);

  tcph->source_port = module__network__data__htons(session->source_port);
  tcph->destination_port = module__network__data__htons(
    session->destination_port);
  tcph->seq = module__network__data__htonl(session->seq);
//  if (flags & module__network__data__ip__tcp_flag__ack)
//  {
//    tcph->ack = module__network__data__htonl(0);//1); //s->recv_seq);
//  }
//  else
//  {
    tcph->ack = module__network__data__htonl(session->ack);
//  }
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

  module__network__data__packet_tcp_checksum(p);
  module__network__data__ip__checksum(p);

//  module_terminal_print_buffer_hex_bytes((char*)(p->buffer), p->length);
  module__network__ethernet_interface__send_packet(p, interface);
  free(p);
}
// -------------------------------------------------------------------------- //
uint8_t module__network__ip__tcp__connect__wait_accept(
  module__network__ethernet_interface * const interface,
  module__network__ip__tcp__session * const session)
{
  // wait for SYN-ACK
//  module_terminal_global_print_c_string("Waiting for TCP SYN/ACK packet on"
//    " driver=");
//  module_terminal_global_print_hex_uint64((uint32_t)(interface->driver));
//  module_terminal_global_print_c_string(" ...\n");
//  module_terminal_global_print_c_string("GOT-0\n");
  while(interface->ipq_index == 0)
  {
//    module_terminal_global_print_c_string("waiting for packet. idx=");
//    module_terminal_global_print_hex_uint64((uint32_t)(interface->ipq_index));
//    module_terminal_global_print_c_string("\n");
    uint8_t *p = NULL;
    *p = 0;
  } // wait for a packet
//  module_terminal_global_print_c_string("GOT-1\n");
  // got a packet ... maybe not the right one ...
  const module__network__data__packet * const p =
    module__network__ethernet_interface__get_packet_from_incoming_queue(
    interface);
//  module_terminal_print_buffer_hex_bytes(p->buffer, 10);//p->length);

  const module__network__data__ethernet_header * const eh =
    module__network__data__packet_get_ethernet_header_const(p);

  if(module_kernel_memcmp(
    &(eh->destination_mac),
    &session->source_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
//    module_terminal_global_print_c_string("Not for my mac.\n");
    return 1;
  }

  if(module_kernel_memcmp(
    &(eh->source_mac),
    &session->destination_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
//    module_terminal_global_print_c_string("Not from expected mac.\n");
    return 2;
  }

  if(module__network__data__ntohs(eh->ethertype)
    != module__network__data__ethernet_header_type__ip_v4)
  {
//    module_terminal_global_print_c_string("Not IPv4.\n");
    return 3;
  }


  const module__network__data__ip_header * const iph =
    module__network__data__packet_get_ip_header_const(p);

  if(iph->version != 4)
  {
//    module_terminal_global_print_c_string("Not IPv4 IP header.\n");
    return 4;
  }

  if(iph->destination_ip != session->source_ip)
  {
//    module_terminal_global_print_c_string("Not sent to my IPv4.\n");
    return 5;
  }

  if(iph->source_ip != session->destination_ip)
  {
//    module_terminal_global_print_c_string("Not sent from server IPv4.\n");
    return 6;
  }


  const module__network__data__ip__tcp_header * const tcph =
    module__network__data__packet_get_ip_tcp_header_const(iph);

  if(module__network__data__ntohs(tcph->source_port) !=
    session->destination_port)
  {
//    module_terminal_global_print_c_string("Not from server port.\n");
    return 7;
  }

  if(module__network__data__ntohs(tcph->destination_port) !=
    session->source_port)
  {
//    module_terminal_global_print_c_string("Not for my port.\n");
    return 8;
  }

  if(!tcph->f_syn || !tcph->f_ack)
  {
//    module_terminal_global_print_c_string("Not SYN & ACK.\n");
    return 9;
  }

  if( module__network__data__ntohl(tcph->ack) != (session->seq + 1) )
  {
    // the server should acknowledge the SYN packet I sent to initiate the
    //session
    return 10;
  }

  // use server's seq as a starting point for the acknowledgement number
  session->ack = module__network__data__ntohl(tcph->seq);
  // acknowledge server's SYN packet ( 1 byte )
  session->ack += 1;

//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));
  free(p);
  return 0;
}
// -------------------------------------------------------------------------- //
void module__network__ip__tcp__connect__acknowledge_accept(
    module__network__ethernet_interface * const interface,
    module__network__ip__tcp__session * const session)
{
  // send ACK packet
  const size_t len = 0;
  const uint8_t flags = module__network__data__ip__tcp_flag__ack;
//  module_terminal_global_print_c_string("TCP conn ACK packet.\n");
  module__network__data__packet *p = module__network__data__packet__alloc();
  p->length = sizeof(module__network__data__ethernet_header)
    + sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len
  ;

  module__network__data__ethernet_header * eh =
    module__network__data__packet_get_ethernet_header(p);
  eh->destination_mac = session->destination_mac;
  eh->source_mac = session->source_mac;
  eh->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__ip_v4);

  module__network__data__ip_header * iph =
    module__network__data__packet_get_ip_header(p);
  iph->version = 4;
  iph->header_length = 5;
  iph->dscp = 0;
  iph->total_length = module__network__data__htons(
    sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len);
  iph->id = module__network__data__htons(0);
  iph->flags_frag = module__network__data__htons(1 << 14); // Don't Fragment bit
  iph->ttl = 64;
  iph->protocol = module__network__data__ethernet__ip__protocol_type__tcp;
  iph->header_checksum = 0; // populated later at the end, after data
  iph->source_ip = session->source_ip;
  iph->destination_ip = session->destination_ip;

  module__network__data__ip__tcp_header * tcph =
    module__network__data__packet_get_ip_tcp_header(iph);

  tcph->source_port = module__network__data__htons(session->source_port);
  tcph->destination_port = module__network__data__htons(
    session->destination_port);
  tcph->seq = module__network__data__htonl(session->seq); // next seq
//  if (flags & module__network__data__ip__tcp_flag__ack)
//  {
    tcph->ack = module__network__data__htonl(session->ack);//0x0000fa01 + 1);//prev sent seq+1??
//  }
//  else
//  {
//    tcph->ack = 0;
//  }
  tcph->f_ns = 0; // 1 bit
  tcph->_reserved = 0; // 3 bits
  tcph->offset = 5;//6;//10/2;//5; // 5=no_opt & 10=opt // 4 bits
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

  module__network__data__packet_tcp_checksum(p);
  module__network__data__ip__checksum(p);

  //  module_terminal_print_buffer_hex_bytes2((char*)(p->buffer), p->length);
  module__network__ethernet_interface__send_packet(p, interface);
  free(p);
}
// -------------------------------------------------------------------------- //
void module__network__ip__tcp__connect(
  module__network__ethernet_interface * const interface,
  module__network__ip__tcp__session * const session)
{
  /*
  The idea is that two parties want to agree on a set of parameters, which, in
  the case of opening a TCP connection, are the starting sequence numbers the
  two sides plan to use for their respective byte streams. In general, the
  parameters might be any facts that each side wants the other to know about.

  First, the client (the active participant) sends a segment to the server (the
  passive participant) stating the initial sequence number it plans to use
  (Flags = SYN, SequenceNum = x). The server then responds with a single segment
  that both acknowledges the client's sequence number (Flags = ACK, Ack = x + 1)
  and states its own beginning sequence number (Flags = SYN, SequenceNum = y).
  That is, both the SYN and ACK bits are set in the Flags field of this second
  message.

  Finally, the client responds with a third segment that acknowledges the
  server's sequence number (Flags = ACK, Ack = y + 1). The reason why each side
  acknowledges a sequence number that is one larger than the one sent is that
  the Acknowledgment field actually identifies the “next sequence number
  expected,” thereby implicitly acknowledging all earlier sequence numbers.
  Although not shown in this timeline, a timer is scheduled for each of the
  first two segments, and if the expected response is not received the segment
  is retransmitted.

  You may be asking yourself why the client and server have to exchange starting
  sequence numbers with each other at connection setup time. It would be simpler
  if each side simply started at some “well-known” sequence number, such as 0.
  In fact, the TCP specification requires that each side of a connection select
  an initial starting sequence number at random. The reason for this is to
  protect against two incarnations of the same connection reusing the same
  sequence numbers too soon—that is, while there is still a chance that a
  segment from an earlier incarnation of a connection might interfere with a
  later incarnation of the connection.
   */
  module__network__ip__tcp__connect__init(interface, session);
  session->connection_state =
    module__network__ip__tcp__session__connection_state__initiated;

  uint8_t r = 1;
  r = module__network__ip__tcp__connect__wait_accept(interface, session);
  if(r != 0)
  {
    module_terminal_global_print_c_string("error while connecting to TCP server"
      "! err=");
    module_terminal_global_print_uint64(r);
    module_terminal_global_print_c_string("\n");
    return;
  }
  session->connection_state =
    module__network__ip__tcp__session__connection_state__accepted;

  session->seq += 1; // send the next packet
  module__network__ip__tcp__connect__acknowledge_accept(interface, session);
  session->connection_state =
    module__network__ip__tcp__session__connection_state__connected;
}
// -------------------------------------------------------------------------- //
uint8_t module__network__ip__tcp__send__wait_acknowledge(
  module__network__ethernet_interface * const interface,
  module__network__ip__tcp__session * const session)
{
  // wait for ACK of sent payload data
//  module_terminal_global_print_c_string("Waiting for TCP SYN/ACK packet on"
//    " driver=");
//  module_terminal_global_print_hex_uint64((uint32_t)(interface->driver));
//  module_terminal_global_print_c_string(" ...\n");
//  module_terminal_global_print_c_string("GOT-0\n");
  while(interface->ipq_index == 0)
  {
//    module_terminal_global_print_c_string("waiting for packet. idx=");
//    module_terminal_global_print_hex_uint64((uint32_t)(interface->ipq_index));
//    module_terminal_global_print_c_string("\n");
    uint8_t *p = NULL;
    *p = 0;
  } // wait for a packet
//  module_terminal_global_print_c_string("GOT-1\n");
  // got a packet ... maybe not the right one ...
  const module__network__data__packet * const p =
    module__network__ethernet_interface__get_packet_from_incoming_queue(
    interface);
//  module_terminal_print_buffer_hex_bytes(p->buffer, 10);//p->length);

  const module__network__data__ethernet_header * const eh =
    module__network__data__packet_get_ethernet_header_const(p);

  if(module_kernel_memcmp(
    &(eh->destination_mac),
    &session->source_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
//    module_terminal_global_print_c_string("Not for my mac.\n");
    return 1;
  }

  if(module_kernel_memcmp(
    &(eh->source_mac),
    &session->destination_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
//    module_terminal_global_print_c_string("Not from expected mac.\n");
    return 2;
  }

  if(module__network__data__ntohs(eh->ethertype)
    != module__network__data__ethernet_header_type__ip_v4)
  {
//    module_terminal_global_print_c_string("Not IPv4.\n");
    return 3;
  }


  const module__network__data__ip_header * const iph =
    module__network__data__packet_get_ip_header_const(p);

  if(iph->version != 4)
  {
//    module_terminal_global_print_c_string("Not IPv4 IP header.\n");
    return 4;
  }

  if(iph->destination_ip != session->source_ip)
  {
//    module_terminal_global_print_c_string("Not sent to my IPv4.\n");
    return 5;
  }

  if(iph->source_ip != session->destination_ip)
  {
//    module_terminal_global_print_c_string("Not sent from server IPv4.\n");
    return 6;
  }


  const module__network__data__ip__tcp_header * const tcph =
    module__network__data__packet_get_ip_tcp_header_const(iph);

  if(module__network__data__ntohs(tcph->source_port) !=
    session->destination_port)
  {
//    module_terminal_global_print_c_string("Not from server port.\n");
    return 7;
  }

  if(module__network__data__ntohs(tcph->destination_port) !=
    session->source_port)
  {
//    module_terminal_global_print_c_string("Not for my port.\n");
    return 8;
  }

  if(!tcph->f_ack)
  {
//    module_terminal_global_print_c_string("Not ACK.\n");
    return 9;
  }

  if( module__network__data__ntohl(tcph->ack) != (session->seq) )
  {
    // the server should acknowledge the SYN packet I sent to initiate the
    //session
    return 10;
  }

  // acknowledge server's seq
  session->ack = module__network__data__ntohl(tcph->seq) + 1;

//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));
  free(p);
  return 0;
}
// -------------------------------------------------------------------------- //
uint8_t module__network__ip__tcp__send(
  module__network__ethernet_interface * const interface,
  module__network__ip__tcp__session * const session,
  const char * const buffer, const size_t buffer_size)
{
  const uint16_t len = buffer_size; // split in multiple packets
  const uint8_t flags = module__network__data__ip__tcp_flag__psh
    | module__network__data__ip__tcp_flag__ack;
//  module_terminal_global_print_c_string("Sending HTTP request.\n");
  module__network__data__packet *p = module__network__data__packet__alloc();
  p->length = sizeof(module__network__data__ethernet_header)
    + sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len
  ;

  module__network__data__ethernet_header * eh =
    module__network__data__packet_get_ethernet_header(p);
  eh->destination_mac = session->destination_mac;
  eh->source_mac = session->source_mac;
  eh->ethertype = module__network__data__htons(
    module__network__data__ethernet_header_type__ip_v4);

  module__network__data__ip_header * iph =
    module__network__data__packet_get_ip_header(p);
  iph->version = 4;
  iph->header_length = 5;
  iph->dscp = 0;
  iph->total_length = module__network__data__htons(
    sizeof(module__network__data__ip_header)
    + sizeof(module__network__data__ip__tcp_header)
    + len);
  iph->id = module__network__data__htons(0);
  iph->flags_frag = module__network__data__htons(1 << 14); // Don't Fragment bit
  iph->ttl = 64;
  iph->protocol = module__network__data__ethernet__ip__protocol_type__tcp;
  iph->header_checksum = 0; // populated later at the end, after data
  iph->source_ip = session->source_ip;
  iph->destination_ip = session->destination_ip;

  module__network__data__ip__tcp_header * tcph =
    module__network__data__packet_get_ip_tcp_header(iph);

  tcph->source_port = module__network__data__htons(session->source_port);
  tcph->destination_port = module__network__data__htons(
    session->destination_port);
  tcph->seq = module__network__data__htonl(session->seq);
//  if (flags & module__network__data__ip__tcp_flag__ack)
//  {
//    tcph->ack = module__network__data__htonl(0);
//  }
//  else
//  {
    tcph->ack = module__network__data__htonl(session->ack);
//  }
  tcph->f_ns = 0; // 1 bit
  tcph->_reserved = 0; // 3 bits
  tcph->offset = 5;//6;//10/2;//5; // 5=no_opt & 10=opt // 4 bits
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

  module_kernel_memcpy(buffer, tcph->data, buffer_size);

  module__network__data__packet_tcp_checksum(p);
  module__network__data__ip__checksum(p);

//  module_terminal_print_buffer_hex_bytes((uint8_t*)(p->buffer), p->length);
  module__network__ethernet_interface__send_packet(p, interface);
  free(p);
  session->seq += buffer_size;
  uint8_t r = 1;
  r = module__network__ip__tcp__send__wait_acknowledge(interface, session);
  return r;
}
// -------------------------------------------------------------------------- //
uint8_t module__network__ip__tcp__recv(
  module__network__ethernet_interface * const interface,
  module__network__ip__tcp__session * const session,
  char * const buffer, const size_t buffer_size, size_t * const bytes_read)
{
  // wait for ACK of sent payload data
//  module_terminal_global_print_c_string("Waiting for TCP SYN/ACK packet on"
//    " driver=");
//  module_terminal_global_print_hex_uint64((uint32_t)(interface->driver));
//  module_terminal_global_print_c_string(" ...\n");
//  module_terminal_global_print_c_string("GOT-0\n");
  while(interface->ipq_index == 0)
  {
//    module_terminal_global_print_c_string("waiting for packet. idx=");
//    module_terminal_global_print_hex_uint64((uint32_t)(interface->ipq_index));
//    module_terminal_global_print_c_string("\n");
    uint8_t *p = NULL;
    *p = 0;
  } // wait for a packet
//  module_terminal_global_print_c_string("GOT-1\n");
  // got a packet ... maybe not the right one ...
  const module__network__data__packet * const p =
    module__network__ethernet_interface__get_packet_from_incoming_queue(
    interface);
//  module_terminal_print_buffer_hex_bytes(p->buffer, 10);//p->length);

  const module__network__data__ethernet_header * const eh =
    module__network__data__packet_get_ethernet_header_const(p);

  if(module_kernel_memcmp(
    &(eh->destination_mac),
    &session->source_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
//    module_terminal_global_print_c_string("Not for my mac.\n");
    return 1;
  }

  if(module_kernel_memcmp(
    &(eh->source_mac),
    &session->destination_mac,
    sizeof(module__network__data__mac_address))
    != 0)
  {
//    module_terminal_global_print_c_string("Not from expected mac.\n");
    return 2;
  }

  if(module__network__data__ntohs(eh->ethertype)
    != module__network__data__ethernet_header_type__ip_v4)
  {
//    module_terminal_global_print_c_string("Not IPv4.\n");
    return 3;
  }


  const module__network__data__ip_header * const iph =
    module__network__data__packet_get_ip_header_const(p);

  if(iph->version != 4)
  {
//    module_terminal_global_print_c_string("Not IPv4 IP header.\n");
    return 4;
  }

  if(iph->destination_ip != session->source_ip)
  {
//    module_terminal_global_print_c_string("Not sent to my IPv4.\n");
    return 5;
  }

  if(iph->source_ip != session->destination_ip)
  {
//    module_terminal_global_print_c_string("Not sent from server IPv4.\n");
    return 6;
  }


  const module__network__data__ip__tcp_header * const tcph =
    module__network__data__packet_get_ip_tcp_header_const(iph);

  if(module__network__data__ntohs(tcph->source_port) !=
    session->destination_port)
  {
//    module_terminal_global_print_c_string("Not from server port.\n");
    return 7;
  }

  if(module__network__data__ntohs(tcph->destination_port) !=
    session->source_port)
  {
//    module_terminal_global_print_c_string("Not for my port.\n");
    return 8;
  }

  if(!tcph->f_ack)
  {
//    module_terminal_global_print_c_string("Not ACK.\n");
    return 9;
  }

  if( module__network__data__ntohl(tcph->ack) != (session->seq) )
  {
    // the server should acknowledge the SYN packet I sent to initiate the
    //session
    return 10;
  }

  const uint16_t tcph_payload_size =
    module__network__data__ntohs(iph->total_length)
    - sizeof(module__network__data__ip_header)
    - sizeof(module__network__data__ip__tcp_header);
  module_terminal_global_print_c_string("Received TCP payload=");
  module_terminal_global_print_uint64(tcph_payload_size);
  module_terminal_global_print_c_string("\n");

  module_kernel_memcpy(tcph->data, buffer, tcph_payload_size);
//  module_terminal_print_buffer_hex_bytes(p->buffer, (size_t)(p->length));
//  module__network__data__packet_print_ip_udp_bootp_header(
//    module__network__data__packet_get_ip_udp_bootp_header_const(
//    module__network__data__packet_get_ip_udp_header_const(
//    module__network__data__packet_get_ip_header_const(p))));
  free(p);
  *bytes_read += tcph_payload_size;
  // acknowledge bytes received from server
  session->ack += tcph_payload_size;

  module__network__ip__tcp__connect__acknowledge_accept(interface, session);
  return 0;
}
// -------------------------------------------------------------------------- //
void module__network__ip__tcp__test(
  module__network__ethernet_interface * const interface,
  const module__network__data__mac_address server_mac, const uint32_t server_ip,
  const uint16_t server_port)
{
  module_terminal_global_print_c_string("\n=============\n");

  const uint16_t random_port = 54321;
  module__network__ip__tcp__session s;
  module__network__ip__tcp__session_create(&s, interface->mac_address,
    interface->ip, random_port, server_mac, server_ip, server_port);

  // step 1 = connect
  module__network__ip__tcp__connect(interface, &s);

  // step 2 = send request
  const char * const request =
    "GET / HTTP/1.1\r\n"
    "User-Agent: Wget/1.20.1 (linux-gnu)\r\n"
    "Accept: */*\r\n"
    "Accept-Encoding: identity\r\n"
    "Host: 10.0.2.2:1030\r\n"
    "Connection: Keep-Alive\r\n"
    "\r\n"
    ;
  const size_t request_size = 140;
  uint8_t r = 1;
  r = module__network__ip__tcp__send(interface, &s, request, request_size);
  if(r != 0)
  {
    module_terminal_global_print_c_string("error while sending data to TCP"
      " server! err=");
    module_terminal_global_print_uint64(r);
    module_terminal_global_print_c_string("\n");
    return;
  }

  // step 3 = receive response
  const size_t buffer_size = 1024*1024;
  char buffer[1024*1024];
  size_t bytes_read = 0;
  for (uint8_t i=0;i<1;i++)
  {
    size_t pbr = bytes_read;
    module__network__ip__tcp__recv(interface, &s, buffer, buffer_size,
      &bytes_read);
    module_terminal_global_print_c_string("\n=============\n");
    for (size_t i=pbr;i<pbr+300;i++)
    {
      module_terminal_global_print_char(buffer[i]);
    }
    module_terminal_global_print_c_string("\n=============\n");
  }
  module_terminal_global_print_c_string("\n=============\n");
}
// -------------------------------------------------------------------------- //
