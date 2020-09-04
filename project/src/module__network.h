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
   */
  uint8_t buffer[]; // why not equivalent to "uint8_t * buffer" ?
} module__network__packet;
// -------------------------------------------------------------------------- //
enum module__network__ethernet_packet_type
{
  module__network__ethernet_packet_type__ip = 0x0800,
  module__network__ethernet_packet_type__arp = 0x0806,
};
// -------------------------------------------------------------------------- //
typedef struct __attribute__((__packed__))
{
  uint8_t data[6];
} module__network__mac_address;
// -------------------------------------------------------------------------- //
//! Run short network test
void module__network__test();
void module__network__print_mac(const module__network__mac_address * const ma);
void module__network__process_ethernet_packet(
  const module__network__packet * const p);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

