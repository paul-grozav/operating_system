// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
/**
 * output byte - set byte to port
 * @param[in] port - 16 bits unsigned port, where the value will be set
 * @param[in] value - 8 bits (1 byte) value that will be set
 */
void outb(const uint16_t port, const uint8_t value);

/**
 * input byte - read byte from port
 * @param[in] port - 16 bits unsigned port, where the value we need resides.
 * @return the 8 bits value read from the given port.
 */
uint8_t inb(const uint16_t port);

//! Test communication over serial port
void module_serial_test();
// -------------------------------------------------------------------------- //

