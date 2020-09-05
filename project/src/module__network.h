// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK_H
#define MODULE__NETWORK_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
// -------------------------------------------------------------------------- //
typedef struct
{
//  struct net_device *from;
//  void* queue;
//  int refcount;
//  uint8_t user_anno[32];

  /**
   * Number of bytes of buffer data, following this struct in memory.
   */
  int32_t length; // -1 if unknown

  /**
   * Sequence of bytes following this structure, in the allocated heap zone.
   * These bytes for an ethernet_packet
   */
  uint8_t buffer[]; // why not equivalent to "uint8_t * buffer" ?
} module__network__packet;
// -------------------------------------------------------------------------- //
enum module__network__ethernet_header_type
{
  module__network__ethernet_header_type__ip = 0x0800,
  module__network__ethernet_header_type__arp = 0x0806,
};
// -------------------------------------------------------------------------- //
typedef struct __attribute__((__packed__))
{
  uint8_t data[6];
} module__network__mac_address;
// -------------------------------------------------------------------------- //
typedef struct __attribute__((__packed__))
{
  module__network__mac_address destination_mac;
  module__network__mac_address source_mac;
  uint16_t ethertype;
  uint8_t buffer[];
} module__network__ethernet_header;
// -------------------------------------------------------------------------- //
typedef struct __attribute__((__packed__))
{
  // eth_hdr
  uint16_t hw_type;
  uint16_t proto;
  uint8_t hw_size;
  uint8_t proto_size;

  //! ARP operation-request or response
  uint16_t operation_type;

  //! The MAC address of the sender machine
  module__network__mac_address sender_mac;

  //! The IP address of the sender machine
  uint32_t sender_ip;

  //! The MAC address of the target machine
  module__network__mac_address target_mac;

  //! The IP address of the target machine
  uint32_t target_ip;
} module__network__arp_header;
// -------------------------------------------------------------------------- //
enum module__network__ethernet__arp_operation_type
{
  //! ARP request
  network__ethernet__arp_operation_type__request = 1,

  //! ARP response
  network__ethernet__arp_operation_type__response = 2
};
// -------------------------------------------------------------------------- //
//! Run short network test
void module__network__test();
void module__network__print_mac(const module__network__mac_address * const ma);
void module__network__print_ip(const uint32_t ip);
void module__network__process_ethernet_packet(
  const module__network__packet * const p);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

