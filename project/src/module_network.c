// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include "module_network.h"
#include "module_kernel.h"
#include "module_terminal.h"
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
#define outportb(a,b) module_kernel_out_8(a,b)
#define inportb(a,b) module_kernel_in_8(a,b)
uint32_t vendorID = 0xFFFF;
 void checkDevice(uint8_t bus, uint8_t device) {
     uint8_t function = 0;
 
     vendorID = getVendorID(bus, device, function);
     if(vendorID == 0xFFFF) return;        // Device doesn't exist
     checkFunction(bus, device, function);
     headerType = getHeaderType(bus, device, function);
     if( (headerType & 0x80) != 0) {
         /* It is a multi-function device, so check remaining functions */
         for(function = 1; function < 8; function++) {
             if(getVendorID(bus, device, function) != 0xFFFF) {
                 checkFunction(bus, device, function);
             }
         }
     }
 }
 
void checkFunction(uint8_t bus, uint8_t device, uint8_t function)
{
  module_terminal_global_print_c_string("bus:");
  module_terminal_global_print_uint64(bus);
  module_terminal_global_print_c_string(", device:");
  module_terminal_global_print_uint64(device);
  module_terminal_global_print_c_string(", function:");
  module_terminal_global_print_uint64(function);
  module_terminal_global_print_c_string(".\n");
}
 void checkAllBuses(void) {
     uint16_t bus;
     uint8_t device;
 
     for(bus = 0; bus < 256; bus++) {
         for(device = 0; device < 32; device++) {
             checkDevice(bus, device);
         }
     }
 }
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
void module_network_test()
{
  // https://wiki.osdev.org/RTL8139

  checkAllBuses();
/*
  // Turn on
  module_terminal_global_print_c_string("Network: Turn on");
  outportb( ioaddr + 0x52, 0x0);

  // software reset
  module_terminal_global_print_c_string("Network: Software reset");
  outportb( ioaddr + 0x37, 0x10);
  module_terminal_global_print_c_string("Network: Wait");
  while( (inb(ioaddr + 0x37) & 0x10) != 0) { }
  module_terminal_global_print_c_string("Network: Test done");
*/
}
// -------------------------------------------------------------------------- //

