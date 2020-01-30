// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include <stddef.h> // size_t
// -------------------------------------------------------------------------- //
/**
 * output byte - set byte to port
 * @param[in] port - 16 bits unsigned port, where the value will be set
 * @param[in] value - 8 bits (1 byte) value that will be set
 */
void module_kernel_out_byte(const uint16_t port, const uint8_t value);

/**
 * input byte - read byte from port
 * @param[in] port - 16 bits unsigned port, where the value we need resides.
 * @return the 8 bits value read from the given port.
 */
uint8_t module_kernel_in_byte(const uint16_t port);

/**
 * Set memory range to constant value.
 * Sets all memory bytes from [start] to contain the [value] value. [length]
 * bytes will be populated, starting from [start].
 * @param[in] start - address of first byte to set
 * @param[in] value - byte value to assign to all bytes in given range
 * @param[in] length - number of bytes to overwrite
 * @note All values in that interval will be lost!
 */
void module_kernel_memset(void *start, const char value, const size_t length);
// -------------------------------------------------------------------------- //

