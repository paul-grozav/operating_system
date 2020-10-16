// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__IP__TCP_H
#define MODULE__NETWORK__IP__TCP_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX
//#include <stddef.h> // size_t
#include "module__network__data.h"
#include "module__network__ethernet_interface.h"
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
enum module__network__ip__tcp__session__connection_state
{
  //! connection object just created - did not attempt to connect it yet.
  module__network__ip__tcp__session__connection_state__not_initiated = 0,

  // Attempt connecting

  //! SYN packet (connection attempt) sent, awaiting SYN-ACK
  module__network__ip__tcp__session__connection_state__initiated = 1,
  //! Got the SYN-ACK (server accepted the connection)
  module__network__ip__tcp__session__connection_state__accepted = 2,
  //! Sent the ACK, acknowledge the server acceptance. Connected at this point.
  module__network__ip__tcp__session__connection_state__connected = 3,

};

// -------------------------------------------------------------------------- //
typedef struct
{
  module__network__data__mac_address source_mac;
  uint32_t source_ip;
  uint16_t source_port;

  module__network__data__mac_address destination_mac;
  uint32_t destination_ip;
  uint16_t destination_port;

  enum module__network__ip__tcp__session__connection_state connection_state;

  /**
   * Number of packets, previously sent, that were carrying payload.
   * @note The initial SYN packet, sent to initiate a TCP connection is going to
   * increment this seq number. But the ACK sent after the connection attempt
   * was accepted, will not increment this sequence number.
   * @note a.k.a. "bytes_sent"
   */
  uint32_t seq;

  /**
   * Acknowledgement is set to (the_seq_of_the_last_received_packet + 1)
   */
  uint32_t ack;

} module__network__ip__tcp__session;
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
void module__network__ip__tcp__session_create(
  module__network__ip__tcp__session * const session,
  const module__network__data__mac_address source_mac,
  const uint32_t source_ip,
  const uint16_t source_port,
  const module__network__data__mac_address destination_mac,
  const uint32_t destination_ip,
  const uint16_t destination_port
  );
// -------------------------------------------------------------------------- //
//! Run a short TCP test
void module__network__ip__tcp__test(
  module__network__ethernet_interface * const interface,
  const module__network__data__mac_address server_mac, const uint32_t server_ip,
  const uint16_t server_port);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
