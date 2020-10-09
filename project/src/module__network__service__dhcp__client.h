// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__SERVICE__DHCP__CLIENT_H
#define MODULE__NETWORK__SERVICE__DHCP__CLIENT_H
// -------------------------------------------------------------------------- //
#include <stdbool.h> // bool
#include "module__network__ethernet_interface.h"
// -------------------------------------------------------------------------- //
//! Get network card configuration from DHCP server in LAN
bool module__network__service__dhcp__client__get_net_config(
  module__network__ethernet_interface * const interface,
  module__network__data__dhcp_config * const cfg);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
