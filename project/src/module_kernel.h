// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE_KERNEL_H
#define MODULE_KERNEL_H
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include <stddef.h> // size_t
// -------------------------------------------------------------------------- //
// Read/Write an 8/16/32 bit unsigned integer value on a given I/O location as
// port.
// -------------------------------------------------------------------------- //
/**
 * output byte - set byte to hardware port
 * @param[in] port - 16 bits unsigned port, where the value will be set
 * @param[in] value - 8 bits (1 byte) value that will be set
 * @note this is an implementation of outportb
 */
void module_kernel_out_8(const uint16_t port, const uint8_t value);

/**
 * input byte - read byte from hardware port
 * @param[in] port - 16 bits unsigned port, where the value we need resides.
 * @return the unsigned 8 bits value read from the given port.
 * @note this is an implementation of inportb
 */
uint8_t module_kernel_in_8(const uint16_t port);

/**
 * Write a 16 bit unsigned integer value to the given hardware port.
 * @param[in] port - 16 bits unsigned port, where the value will be set
 * @param[in] value - 16 bits (2 bytes) unsigned integer value that will be set.
 * @note this is an implementation of outportw
 */
void module_kernel_out_16(const uint16_t port, const uint16_t value);

/**
 * Read a 16 bit unsigned integer from the given hardware port
 * @param[in] port - 16 bits unsigned port, where the value we need resides.
 * @return the unsigned 16 bits value read from the given port.
 * @note this is an implementation of inportw
 */
uint16_t module_kernel_in_16(const uint16_t port);

/**
 * Write a 32 bit unsigned integer value to the given hardware port.
 * @param[in] port - 16 bits unsigned port, where the value will be set
 * @param[in] value - 32 bits (4 bytes) unsigned integer value that will be set.
 * @note this is an implementation of outportl
 */
void module_kernel_out_32(const uint16_t port, const uint32_t value);

/**
 * Read a 32 bit unsigned integer from the given hardware port
 * @param[in] port - 16 bits unsigned port, where the value we need resides.
 * @return the unsigned 32 bits value read from the given port.
 * @note this is an implementation of inportl
 */
uint32_t module_kernel_in_32(const uint16_t port);

// -------------------------------------------------------------------------- //
// MEMORY STUFF
// -------------------------------------------------------------------------- //
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
/**
 * Copy memory from source to destination
 * @param[in, out] source - Pointer to source data buffer. Data will be read
 * from here.
 * @param[in, out] destination - Pointer to destination data. Data will be
 * written here.
 * @param[in] size - Number of bytes to copy from source to destination buffer.
 */
void module_kernel_memcpy(const void * const source, void* destination,
  const size_t size);
// -------------------------------------------------------------------------- //
/**
 * Copy memory from source to destination
 * @param[in, out] a - Pointer to first data buffer. Data will be read from
 * here.
 * @param[in, out] b - Pointer to second data buffer. Data will be read from
 * here.
 * @param[in] size - Number of bytes to compare the two data buffers.
 * @return If all the first size bytes are equal in the a buffer and b buffer,
 * it will return 0. At the first byte that differs, if the byte from a has a
 * numeric value less than the byte from b, then it will return -1. If the byte
 * from a has a numeric value greater than the byte from b, then it will return
 * 1.
 */
int8_t module_kernel_memcmp(const void * const a, const void * const b,
  const size_t size);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //
