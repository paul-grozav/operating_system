// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include <stddef.h> // size_t
// -------------------------------------------------------------------------- //
typedef unsigned int uintptr __attribute__ ((mode (pointer)));
// -------------------------------------------------------------------------- //
typedef struct module_heap_struct_heap_block_bm
{
  struct module_heap_struct_heap_block_bm *next;
  uint32_t size;
  uint32_t used;
  uint32_t bsize;
  uint32_t lfb;
} module_heap_heap_block_bm;
// -------------------------------------------------------------------------- //
typedef struct module_heap_struct_heap_bm
{
  module_heap_heap_block_bm *fblock;
} module_heap_heap_bm;
// -------------------------------------------------------------------------- //
/**
 * bytes will be populated, starting from [start].
 * @param[in] length - number of bytes to overwrite
 * @note All values in that interval will be lost!
 */
//void module_kernel_memset(void *start, const char value, const size_t length);

void k_heapBMInit(module_heap_heap_bm *heap);

int k_heapBMAddBlock(module_heap_heap_bm *heap, uintptr addr, uint32_t size,
  uint32_t bsize);

void *k_heapBMAlloc(module_heap_heap_bm *heap, uint32_t size);

void k_heapBMFree(module_heap_heap_bm *heap, void *ptr);
// -------------------------------------------------------------------------- //

