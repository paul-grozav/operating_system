// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__DATA_H
#define MODULE__NETWORK__DATA_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
#include <stddef.h> // size_t
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Utils
// -------------------------------------------------------------------------- //
//! these were "static inline" in .c file
uint16_t module__network__data__ntohs(const uint16_t x);
// -------------------------------------------------------------------------- //
uint16_t module__network__data__htons(const uint16_t x);
// -------------------------------------------------------------------------- //
uint32_t module__network__data__ntohl(const uint32_t x);
// -------------------------------------------------------------------------- //
uint32_t module__network__data__htonl(const uint32_t x);
// -------------------------------------------------------------------------- //
//! From human readable format, to human readable format
uint32_t module__network__data__ip(const uint8_t a, const uint8_t b,
  const uint8_t c, const uint8_t d);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Packet
// -------------------------------------------------------------------------- //
typedef struct
{
//  struct net_device *from;
//  void* queue;
//  int refcount;
//  uint8_t user_anno[32];

  /**
   * Number of bytes of buffer data, following this struct in memory.
   * @note 0 is an invalid value, unknown, uninitialized.
   */
  uint32_t length;

  /**
   * Sequence of bytes following this structure, in the allocated heap zone.
   * These bytes for an ethernet_packet.
   * @note why not equivalent to "uint8_t * buffer" ?
   */
  uint8_t buffer[];
} module__network__data__packet;
// -------------------------------------------------------------------------- //
module__network__data__packet * module__network__data__packet__alloc();
// -------------------------------------------------------------------------- //
module__network__data__packet * module__network__data__packet__create_with_data(
  const char * const data, const uint32_t length);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Ethernet header
// -------------------------------------------------------------------------- //
//! Enumerates ethernet header types
enum module__network__data__ethernet_header_type
{
  //! An IP header follows the ETH header
  module__network__data__ethernet_header_type__ip_v4 = 0x0800,

  //! An ARP header follows the ETH header
  module__network__data__ethernet_header_type__arp = 0x0806,

  //! An ARP header follows the ETH header
  module__network__data__ethernet_header_type__ip_v6 = 0x86dd,
};
// -------------------------------------------------------------------------- //
//! A 6-byte MAC address
typedef struct __attribute__((__packed__))
{
  //! 6 bytes, which when printed will be represented each as a hex number.
  uint8_t data[6];
} module__network__data__mac_address;
// -------------------------------------------------------------------------- //
extern const module__network__data__mac_address
  module__network__data__mac_address__broadcast_mac;
extern const module__network__data__mac_address
  module__network__data__mac_address__zero_mac;
// -------------------------------------------------------------------------- //
/**
 * Should be present in any ethernet packet.
 */
typedef struct __attribute__((__packed__))
{
  //! The MAC address of the destination machine
  module__network__data__mac_address destination_mac;

  //! The MAC address of the source machine
  module__network__data__mac_address source_mac;

  //! Type of ethernet packet/protocol: ARP, IP, IPv6, etc.
  uint16_t ethertype;

  //! The following data using the ethertype protocol.
  uint8_t buffer[];
} module__network__data__ethernet_header;
// -------------------------------------------------------------------------- //
module__network__data__ethernet_header *
  module__network__data__packet_get_ethernet_header(
  module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
const module__network__data__ethernet_header *
  module__network__data__packet_get_ethernet_header_const(
  const module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
void module__network__data__print_mac(
  const module__network__data__mac_address * const ma);
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ethernet_header(
  const module__network__data__ethernet_header * const h);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Address Resolution Protocol (ARP) Packet
// -------------------------------------------------------------------------- //
/**
 * This is present after the ETH header, if it's an ARP packet.
 * Request: Who has target_ip tell sender_ip?
 * Response: sender_ip is at sender_mac
 */
typedef struct __attribute__((__packed__))
{
  uint16_t hw_type;
  uint16_t proto;
  uint8_t hw_size;
  uint8_t proto_size;

  //! ARP operation-request or response
  uint16_t operation_type;

  //! The MAC address of the sender machine
  module__network__data__mac_address sender_mac;

  //! The IP address of the sender machine
  uint32_t sender_ip;

  //! The MAC address of the target machine
  module__network__data__mac_address target_mac;

  //! The IP address of the target machine.
  uint32_t target_ip;

  // Nothing else follows the ARP header. The packet should end here.
} module__network__data__arp_header;
// -------------------------------------------------------------------------- //
enum module__network__data__ethernet__arp_operation_type
{
  //! ARP request
  module__network__data__ethernet__arp_operation_type__request = 1,

  //! ARP response
  module__network__data__ethernet__arp_operation_type__response = 2
};
// -------------------------------------------------------------------------- //
module__network__data__arp_header *
  module__network__data__packet_get_arp_header(
  module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
const module__network__data__arp_header *
  module__network__data__packet_get_arp_header_const(
  const module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
void module__network__data__arp__header_print(
  const module__network__data__arp_header * const h);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Internet Protocol (IP) Packet
// -------------------------------------------------------------------------- //
/**
 * This is present after the ETH header, if it's an IP packet.
 * See: https://en.wikipedia.org/wiki/IPv4#Packet_structure
 */
typedef struct __attribute__((__packed__))
{
  /**
   * The IPv4 header is variable in size due to the optional 14th field
   * (options). The IHL field contains the size of the IPv4 header, it has 4
   * bits that specify the number of 32-bit words in the header. The minimum
   * value for this field is 5,[28] which indicates a length of
   * 5 × 32 bits = 160 bits = 20 bytes. As a 4-bit field, the maximum value is
   * 15, this means that the maximum size of the IPv4 header is 15 × 32 bits,
   * or 480 bits = 60 bytes.
   *
   * The header_length and version fields form a byte. A value of 0x45 (base 16)
   * means 0b01000101 which is version = 0b0100 = 4 and
   * header_length = 0b0101 = 5 .
   */
  uint8_t header_length : 4;

  /**
   * The first header field in an IP packet is the four-bit version field. For
   * IPv4, this is always equal to 4.
   */
  uint8_t version : 4;

  /**
   * 6 bits for DSCP + 2 bits for ECN
   *
   * Differentiated Services Code Point (DSCP) - 6 bits
   * Originally defined as the type of service (ToS), this field specifies
   * differentiated services (DiffServ) per RFC 2474 (updated by RFC 3168 and
   * RFC 3260). New technologies are emerging that require real-time data
   * streaming and therefore make use of the DSCP field. An example is Voice
   * over IP (VoIP), which is used for interactive voice services.
   *
   * Explicit Congestion Notification (ECN) - 2 bits
   * This field is defined in RFC 3168 and allows end-to-end notification of
   * network congestion without dropping packets. ECN is an optional feature
   * that is only used when both endpoints support it and are willing to use it.
   * It is effective only when supported by the underlying network.
   */
  uint8_t dscp;

  /**
   * This 16-bit field defines the entire packet size in bytes, including header
   * and data. The minimum size is 20 bytes (header without data) and the
   * maximum is 65,535 bytes. All hosts are required to be able to reassemble
   * datagrams of size up to 576 bytes, but most modern hosts handle much larger
   * packets. Sometimes links impose further restrictions on the packet size, in
   * which case datagrams must be fragmented. Fragmentation in IPv4 is handled
   * in either the host or in routers.
   */
  uint16_t total_length;

  /**
   * This field is an identification field and is primarily used for uniquely
   * identifying the group of fragments of a single IP datagram. Some
   * experimental work has suggested using the ID field for other purposes, such
   * as for adding packet-tracing information to help trace datagrams with
   * spoofed source addresses,[29] but RFC 6864 now prohibits any such use.
   */
  uint16_t id;

  /**
   * 3 bits for flags and 13 bits for fragment offset.
   *
   * A three-bit field follows and is used to control or identify fragments.
   * They are (in order, from most significant to least significant):
   * bit 0: Reserved; must be zero. See: https://en.wikipedia.org/wiki/Evil_bit
   * bit 1: Don't Fragment (DF)
   * bit 2: More Fragments (MF)
   * If the DF flag is set, and fragmentation is required to route the packet,
   * then the packet is dropped. This can be used when sending packets to a host
   * that does not have resources to handle fragmentation. It can also be used
   * for path MTU discovery, either automatically by the host IP software, or
   * manually using diagnostic tools such as ping or traceroute. For
   * unfragmented packets, the MF flag is cleared. For fragmented packets, all
   * fragments except the last have the MF flag set. The last fragment has a
   * non-zero Fragment Offset field, differentiating it from an unfragmented
   * packet.
   *
   * The fragment offset field is measured in units of eight-byte blocks. It is
   * 13 bits long and specifies the offset of a particular fragment relative to
   * the beginning of the original unfragmented IP datagram. The first fragment
   * has an offset of zero. This allows a maximum offset of
   * (213 – 1) × 8 = 65,528 bytes, which would exceed the maximum IP packet
   * length of 65,535 bytes with the header length included
   * (65,528 + 20 = 65,548 bytes).
   */
  uint16_t flags_frag;

  /**
   * Time to live.
   * An eight-bit time to live field helps prevent datagrams from persisting
   * (e.g. going in circles) on an internet. This field limits a datagram's
   * lifetime. It is specified in seconds, but time intervals less than 1 second
   * are rounded up to 1. In practice, the field has become a hop count—when the
   * datagram arrives at a router, the router decrements the TTL field by one.
   * When the TTL field hits zero, the router discards the packet and typically
   * sends an ICMP Time Exceeded message to the sender. The program traceroute
   * uses these ICMP Time Exceeded messages to print the routers used by packets
   * to go from the source to the destination.
   */
  uint8_t ttl;

  /**
   * Protocol of this IP packet.
   * See: module__network__data__ethernet__ip__protocol_type .
   */
  uint8_t protocol;

  /**
   * The 16-bit IPv4 header checksum field is used for error-checking of the
   * header. When a packet arrives at a router, the router calculates the
   * checksum of the header and compares it to the checksum field. If the values
   * do not match, the router discards the packet. Errors in the data field must
   * be handled by the encapsulated protocol. Both UDP and TCP have checksum
   * fields.
   * When a packet arrives at a router, the router decreases the TTL field.
   * Consequently, the router must calculate a new checksum.
   */
  uint16_t header_checksum;

  //! The IP address of the source machine.
  uint32_t source_ip;

  //! The IP address of the destination machine.
  uint32_t destination_ip;

  //! Data in proto protocol, following this IP header.
  uint8_t data[];
} module__network__data__ip_header;
// -------------------------------------------------------------------------- //
//! Based on: https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
enum module__network__data__ethernet__ip__protocol_type
{
  //! Internet Control Message Protocol
  module__network__data__ethernet__ip__protocol_type__icmp = 1,

  //! Transmission Control Protocol
  module__network__data__ethernet__ip__protocol_type__tcp = 6,

  //! User Datagram Protocol
  module__network__data__ethernet__ip__protocol_type__udp = 17
};
// -------------------------------------------------------------------------- //
/**
 * Create checksum for ip packet.
 * @param[in] p - packet to create the checksum for. The checksum will be saved
 * inside this packet's specific field.
 */
void module__network__data__ip_checksum(module__network__data__packet *p);
// -------------------------------------------------------------------------- //
void module__network__data__print_ipv4(const uint32_t ip);
// -------------------------------------------------------------------------- //
module__network__data__ip_header * module__network__data__packet_get_ip_header(
  module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
const module__network__data__ip_header *
  module__network__data__packet_get_ip_header_const(
  const module__network__data__packet * const p);
// -------------------------------------------------------------------------- //
void module__network__data__ip__checksum(module__network__data__packet *p);
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_header(
  const module__network__data__ip_header * const h);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// User Datagram Protocol (UDP)
// -------------------------------------------------------------------------- //
typedef struct __attribute__((__packed__))
{
  uint16_t source_port;
  uint16_t destination_port;
  uint16_t length;
  uint16_t checksum;

  //! Data following the IP_UDP header
  char data[];
} module__network__data__ip__udp_header;
// -------------------------------------------------------------------------- //
module__network__data__ip__udp_header *
  module__network__data__packet_get_ip_udp_header(
  module__network__data__ip_header * const ip_header);
// -------------------------------------------------------------------------- //
const module__network__data__ip__udp_header *
  module__network__data__packet_get_ip_udp_header_const(
  const module__network__data__ip_header * const ip_header);
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_udp_header(
  const module__network__data__ip__udp_header * const h);
// -------------------------------------------------------------------------- //
void module__network__data__packet_udp_checksum(
  module__network__data__packet *p);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Transmission Control Protocol (TCP) Packet
// -------------------------------------------------------------------------- //
/**
 * This is present after the IP header, if it's an TCP packet.
 * See: https://en.wikipedia.org/wiki/Transmission_Control_Protocol#
 * TCP_segment_structure
 */
enum module__network__data__ip__tcp_flags
{
  module__network__data__ip__tcp_flag__none = 0,
  module__network__data__ip__tcp_flag__urg = 1 << 0,
  module__network__data__ip__tcp_flag__ack = 1 << 1,
  module__network__data__ip__tcp_flag__psh = 1 << 2,
  module__network__data__ip__tcp_flag__rst = 1 << 3,
  module__network__data__ip__tcp_flag__syn = 1 << 4,
  module__network__data__ip__tcp_flag__fin = 1 << 5,
};
// -------------------------------------------------------------------------- //
typedef struct __attribute__((__packed__))
{
  uint16_t source_port;
  uint16_t destination_port;

  /**
   * @brief seq - sequence number
   * Has a dual role:
   *  1. If the SYN flag is set (1), then this is the initial sequence number.
   *    The sequence number of the actual first data byte and the acknowledged
   *    number in the corresponding ACK are then this sequence number plus 1.
   *  2. If the SYN flag is clear (0), then this is the accumulated sequence
   *    number of the first data byte of this segment for the current session.
   */
  uint32_t seq;

  /**
   * @brief ack acknowledgement number
   * If the ACK flag is set then the value of this field is the next sequence
   * number that the sender of the ACK is expecting. This acknowledges receipt
   * of all prior bytes (if any). The first ACK sent by each end acknowledges
   * the other end's initial sequence number itself, but no data.
   */
  uint32_t ack;

  // byte = 0x01 = 0000 0001 = (ns=1, reserved=0, offset=0)
  // byte = 0xf0 = 1111 0000 = (ns=0, reserved=0, offset=15)
  // byte = 0x0e = 0000 1110 = (ns=0, reserved=7, offset=0)
  // byte = 0x50 = 0101 0000 = (ns=0, reserved=0, offset=5)
  uint16_t f_ns : 1;
  uint16_t _reserved : 3;
  uint16_t offset : 4; // aka - header length ?

  //! finish
  uint16_t f_fin : 1; // Least significant bit, index 0, 2^0 bit
  uint16_t f_syn : 1; // index 1, 2^1 bit
  uint16_t f_rst : 1; // index 2, 2^2 bit

  //! push
  uint16_t f_psh : 1; // index 3, 2^3 bit
  uint16_t f_ack : 1; // index 4, 2^4 bit
  uint16_t f_urg : 1; // index 5, 2^5 bit
  uint16_t f_ece : 1; // index 6, 2^6 bit
  uint16_t f_cwr : 1; // index 7, 2^7 bit
  uint16_t window;
  uint16_t checksum;
  uint16_t urg_ptr;

  //! Data following the IP_TCP header
  char data[];
} module__network__data__ip__tcp_header;
// -------------------------------------------------------------------------- //
module__network__data__ip__tcp_header *
  module__network__data__packet_get_ip_tcp_header(
  module__network__data__ip_header * const ip_header);
// -------------------------------------------------------------------------- //
const module__network__data__ip__tcp_header *
  module__network__data__packet_get_ip_tcp_header_const(
  const module__network__data__ip_header * const ip_header);
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_tcp_header(
  const module__network__data__ip__tcp_header * const h);
// -------------------------------------------------------------------------- //
void module__network__data__packet_tcp_checksum(
  module__network__data__packet *p);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Bootstrap Protocol (BOOTP)
// -------------------------------------------------------------------------- //
//! https://tools.ietf.org/html/rfc951
typedef struct __attribute__((__packed__))
{
  /**
   * packet opcode type / message type
   * 1 = Boot request
   * 2 = Boot reply
   */
  uint8_t op_code;

  //! Hardware address type
  uint8_t hardware_address_type;

  //! Hardware address length
  uint8_t hardware_address_length;

  //! Gateway hops
  uint8_t gateway_hops;

  //! Transaction ID
  uint32_t transaction_id;

  //! Seconds since boot began
  uint16_t seconds_since_boot;

  //! BootP flags
  uint16_t flags;

  //! Client IP address
  uint32_t client_ip_address;

  //! Your (client) IP address
  uint32_t your_ip_address;

  //! Next server IP address
  uint32_t server_ip_address;

  //! Gateway (relay agent) IP address
  uint32_t gateway_ip_address;

  //! Client hardware (mac) address - 16 bytes ( mac=6 + padding of 10 )
  uint8_t	client_hardware_address[16];

  //! Server host name
  uint8_t	server_host_name[64];

  //! boot file name
  uint8_t boot_file_name[128];

  //! Vendor-specific area
  uint8_t	vendor_area[64]; // if support_dhcp the 312 else 64
} module__network__data__ip__udp__bootp_header;
// -------------------------------------------------------------------------- //
module__network__data__ip__udp__bootp_header *
  module__network__data__packet_get_ip_udp_bootp_header(
  module__network__data__ip__udp_header * const udp_header);
// -------------------------------------------------------------------------- //
const module__network__data__ip__udp__bootp_header *
  module__network__data__packet_get_ip_udp_bootp_header_const(
  const module__network__data__ip__udp_header * const udp_header);
// -------------------------------------------------------------------------- //
void module__network__data__packet_print_ip_udp_bootp_header(
  const module__network__data__ip__udp__bootp_header * const h);
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Other structures
// -------------------------------------------------------------------------- //
//! info received from a DHCP server, for our DHCP client (this machine)
typedef struct
{
  //! IP address given to client by DHCP server
  uint32_t ip;

  //! subnet mask given by DHCP server to client
  uint32_t subnet_mask;

  //! GateWay IP address given by DHCP server to client(as router)
  uint32_t gw;

  //! DNS (server) IP address given by DHCP server to client
  uint32_t dns;

  //! IP of DHCP server that made that offer
  uint32_t dhcp_server_ip;

  //! mac address of DHCP server that made that offer
  module__network__data__mac_address dhcp_server_mac;

  //! Time (in seconds) until that offer expires, and you have to renew your
  //! configuration.
  uint32_t lease_time_seconds;
} module__network__data__dhcp_config;
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
