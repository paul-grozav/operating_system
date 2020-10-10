// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__SERVICE__HTTP__CLIENT_H
#define MODULE__NETWORK__SERVICE__HTTP__CLIENT_H
// -------------------------------------------------------------------------- //
#include <stdbool.h> // bool
#include "module__network__ethernet_interface.h"
// -------------------------------------------------------------------------- //
//! Get network card configuration from DHCP server in LAN
bool module__network__service__http__client__request_response(
  module__network__ethernet_interface * const interface,
  const uint32_t server_ip, const module__network__data__mac_address server_mac,
  const uint16_t server_port);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
