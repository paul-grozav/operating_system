// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
// All programs need dynamic memory allocation, on the heap.
// You simply give it chunks of memory (by address) where the heap is :-)
// Heap is a linked list of blocks.
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
// -------------------------------------------------------------------------- //
/**
 * A heap block, containing a pointer to the next block
 */
typedef struct module_heap_struct_heap_block_bm
{
  //! Next heap block
  struct module_heap_struct_heap_block_bm *next;

  //! Size of this block
  uint32_t size;

  //! How much memory is used, in this block
  uint32_t used;
  uint32_t bsize;
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
 * Add a block to the given heap.
 * @param[in, out] heap - heap to add the block to
 * @param[in] addr - Address where this block starts in memory
 * @param[in] size - size of block, in bytes
 * @param[in] bsize - default block size, in bytes
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

