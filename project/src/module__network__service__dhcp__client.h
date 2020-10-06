// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE__NETWORK__SERVICE__DHCP__CLIENT_H
#define MODULE__NETWORK__SERVICE__DHCP__CLIENT_H
// -------------------------------------------------------------------------- //
#include "module__network__ethernet_interface.h"
// -------------------------------------------------------------------------- //
//! Get network card configuration from DHCP server in LAN
void module__network__service__dhcp__client__get_net_config(
  module__network__ethernet_interface * const interface);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
