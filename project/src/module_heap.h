// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// All programs need dynamic memory allocation, on the heap.
// You simply give it chunks of memory (by address) where the heap is :-)
// Heap is a linked list of blocks.
// -------------------------------------------------------------------------- //
#ifndef MODULE_HEAP_H
#define MODULE_HEAP_H

#include <stdint.h> // uintX_t
// -------------------------------------------------------------------------- //
/**
 * A heap block, containing a pointer to the next block. This block has a size
 * of "size" bytes and is split into blocks of size "bsize" bytes.
 */
typedef struct module_heap_struct_heap_block_bm
{
  //! Next heap block
  struct module_heap_struct_heap_block_bm *next;

  //! Size of this block
  uint32_t size;

  /**
   * How many blocks are used, in this block. This means that out of all "size"
   * bytes available in this block, "used" * "bsize" bytes are already used.
   * Even if you allocate 1 byte, it would still use an entire block of "bsize"
   * bytes to store your data.
   */
  uint32_t used;

  //! Number of bytes inside a block. The size of the block.
  uint32_t bsize;

  /**
   * This basically points to the space after the most recent allocation in
   * hopes that during it allocation it will be closer to any free blocks.
   */
  uint32_t lfb;
} module_heap_heap_block_bm;
// -------------------------------------------------------------------------- //
/**
 * This structure is our heap, it has a pointer to the first block.
 */
typedef struct module_heap_struct_heap_bm
{
  //! Pointer to first heap block
  module_heap_heap_block_bm *fblock;
} module_heap_heap_bm;
// -------------------------------------------------------------------------- //
/**
 * Sets the heap to an empty one, with no blocks.
 * @param[in, out] heap - heap to reset
 * @note All previous blocks will be lost
 */
void module_heap_init(module_heap_heap_bm *heap);
// -------------------------------------------------------------------------- //
/**
 * Add a block to the given heap. This block will be split into sub-blocks of
 * bsize bytes. This super-block will have ~approx. size/bsize sub-blocks.
 *
 * @note This block will be a structure at addr, then N bytes composing the
 * bitmap which tells us which sub-block(chunk) is used, then the actual N
 * sub-blocks of data, that are returned as pointers. That's why the size
 * parameter must have a minimum value.
 *
 * @note Even if you allocate just 1 byte, it will allocate a chunk of
 * "at least" bsize bytes (in this super-block).
 *
 * @param[in, out] heap - heap to add the block to
 * @param[in] addr - Address where this block starts in memory
 * @param[in] size - size of (super) block, in bytes. Should be a value like:
 * N*(bsize + 1) + sizeof(block struct). 
 * @param[in] bsize - default sub-block(chunk) size, in bytes
 * @note This block is appended to the current heap(linked list).
 */
void module_heap_add_block(module_heap_heap_bm *heap, const uintptr_t addr,
  const uint32_t size, const uint32_t bsize);
// -------------------------------------------------------------------------- //
/**
 * Allocate memory in the heap.
 * @param[in, out] heap - heap where the memory will be used
 * @param[in] size - size of continuous memory that is required.
 * @return the address of the first byte of memory allocated for you. Or zero
 * (a.k.a. NULL) if no memory found.
 */
void *module_heap_alloc(module_heap_heap_bm *heap, const uint32_t size);
// -------------------------------------------------------------------------- //
/**
 * Free memory in the heap.
 * @param[in, out] heap - heap to reset
 * @param[in] ptr - pointer to free. constant pointer to constant "void/unknown"
 */
void module_heap_free(module_heap_heap_bm *heap, const void * const ptr);
// -------------------------------------------------------------------------- //
#endif // header guard
// -------------------------------------------------------------------------- //

