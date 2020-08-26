// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#ifndef MODULE_BASE_H
#define MODULE_BASE_H
// -------------------------------------------------------------------------- //
/**
 * Convert a uint64 to ascii base 10.
 * @param number - a copy of the number, this is altered as the function creates
 * the ascii representation. That's why it's not a constant or a reference.
 * @param output_buffer - A pointer to a buffer of at least 20 characters. This
 * function will populate the buffer with the digits of the given number, but it
 * will not add an extra '\0' to the buffer. You can allocate an extra character
 * and add the '\0' yourself, if you want to print the number as a C-string.
 * @return The number of digits that were written to the output_buffer. This
 * does not include a '\0' or anything else. Only the digits of the number.
 * @note A 20 characters buffer is enough to hold the maximum uint64 numeric
 * value.
 */
size_t uint64_to_ascii_base10(uint64_t number, char * const output_buffer);

/**
 * Convert a uint64 to ascii base 16.
 * @param number - a copy of the number, this is altered as the function creates
 * the ascii representation. That's why it's not a constant or a reference.
 * @param output_buffer - A pointer to a buffer of at least 20 characters. This
 * function will populate the buffer with the digits of the given number, but it
 * will not add an extra '\0' to the buffer. You can allocate an extra character
 * and add the '\0' yourself, if you want to print the number as a C-string.
 * @return The number of digits that were written to the output_buffer. This
 * does not include a '\0' at the end, it will not include a "0x" suffix, or
 * anything else. Only the digits of the number.
 * @note A 20 characters buffer is enough to hold the maximum uint64 numeric
 * value.
 */
size_t uint64_to_ascii_base16(uint64_t number, char * const output_buffer);

/**
 * Convert a uint64 to ascii base 2.
 * @param number - a copy of the number, this is altered as the function creates
 * the ascii representation. That's why it's not a constant or a reference.
 * @param output_buffer - A pointer to a buffer of at least 64 characters. This
 * function will populate the buffer with the digits of the given number, but it
 * will not add an extra '\0' to the buffer. You can allocate an extra character
 * and add the '\0' yourself, if you want to print the number as a C-string.
 * @return The number of digits that were written to the output_buffer. This
 * does not include a '\0' at the end, it will not include a "0b" suffix, or
 * anything else. Only the digits of the number.
 * @note A 64 characters buffer is enough to hold the maximum uint64 numeric
 * value.
 */
size_t uint64_to_ascii_base2(const uint64_t number, char * const output_buffer);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

