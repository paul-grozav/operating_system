// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stddef.h> // size_t
#include "module__network__data.h"
#include "module_kernel.h"
#include "module_heap.h"
// -------------------------------------------------------------------------- //
const module__network__data__mac_address
  module__network__data__mac_address__broadcast_mac =
  {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
// -------------------------------------------------------------------------- //
const module__network__data__mac_address
  module__network__data__mac_address__zero_mac = {{0, 0, 0, 0, 0, 0}};
// -------------------------------------------------------------------------- //





// -------------------------------------------------------------------------- //
// Packet
// -------------------------------------------------------------------------- //
module__network__data__packet * module__network__data__packet__alloc()
{
  const size_t ETH_MTU = 1536;
  const size_t packet_size = sizeof(module__network__data__packet) + ETH_MTU;
  module__network__data__packet * new_pk = malloc(packet_size);
  module_kernel_memset(new_pk, 1, packet_size);
  new_pk->length = -1;
//  new_pk->from = NULL;
//  new_pk->refcount = 1;
  return new_pk;
}
// -------------------------------------------------------------------------- //
module__network__data__packet * module__network__data__packet__create_with_data(
  const char * const data, const size_t length)
{
  const size_t packet_size = sizeof(module__network__data__packet) + length;
  module__network__data__packet * new_pk = malloc(packet_size);
  module_kernel_memcpy(data, new_pk->buffer, length);
  new_pk->length = (int32_t)(length);
  return new_pk;
}





// -------------------------------------------------------------------------- //
// Ethernet header
// -------------------------------------------------------------------------- //
module__network__data__ethernet_header *
  module__network__data__packet_get_ethernet_header(
  module__network__data__packet * const p)
{
  return (module__network__data__ethernet_header *)&(p->buffer);
}
// -------------------------------------------------------------------------- //
