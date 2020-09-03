// -------------------------------------------------------------------------- //
// Author: Tancredi-Paul Grozav <paul@grozav.info>
// -------------------------------------------------------------------------- //
#include <stdint.h> // uintX_t
#include "module_heap.h"
// -------------------------------------------------------------------------- //
// Based on the work of:
//    2014 Leonard Kevin McGuire Jr (www.kmcg3413.net) (kmcg3413@gmail.com)
//    2016 Clément Gallet (provided bug fixes)
//    2019 Fix PGrozav
// -------------------------------------------------------------------------- //
void module_heap_init(module_heap_heap_bm *heap)
{
  heap->fblock = 0;
}
// -------------------------------------------------------------------------- //
void module_heap_add_block(module_heap_heap_bm *heap, const uintptr_t addr,
  const uint32_t size, const uint32_t bsize)
{
  module_heap_heap_block_bm *b; // heap block structure that is currently added
  uint32_t bcnt; // number of sub-blocks(chunks) that will be inside this block
  uint32_t x; // iterates over bytes in bitmap
  uint8_t *bm; // BitMap after the structure in meory

  b = (module_heap_heap_block_bm*)addr;
//  if(size < sizeof_block)
//  {
    // issue error - size should be a N*block_size + sizeof_block
//  }
  b->size = size - sizeof(module_heap_heap_block_bm); // sizeof = 20 bytes
  b->bsize = bsize;

  b->next = heap->fblock;
  heap->fblock = b;

  bcnt = b->size / b->bsize;
  bm = (uint8_t*)&b[1]; // 14 bytes to the right of b. but why 14 and not 20?

  // this bitmap has "block_count" positions of 1 bytes(uint8) each?
  // and having a 0 on position N means that the Nth block is not used, and if
  // in the bitap at position N is a value != it eans that the Nth block is used
  // ?? this is just an assumption

  // clear bitmap
  for (x = 0; x < bcnt; ++x)
  {
    bm[x] = 0;
  }

  // reserve room for bitmap
  bcnt = (bcnt / bsize) * bsize < bcnt ? bcnt / bsize + 1 : bcnt / bsize;
  for (x = 0; x < bcnt; ++x)
  {
    bm[x] = 5; // any non-zero value to mark it as used?
  }

  b->lfb = bcnt - 1;
  b->used = bcnt;
}
// -------------------------------------------------------------------------- //
// find ID that does not match left or right
static uint8_t module_heap_get_nid(uint8_t a, uint8_t b)
{
  uint8_t c;
  for (c = a + 1; c == b || c == 0; ++c);
  return c;
}
// -------------------------------------------------------------------------- //
void *module_heap_alloc(module_heap_heap_bm *heap, const uint32_t size)
{
  module_heap_heap_block_bm *b; // current iterating block
  uint8_t *bm; // pointer to first byte bitmap of current block
  uint32_t bcnt; // count of sub-blocks in current block
  uint32_t x, y, z;
  uint32_t bneed; // how many blocks we need in the current block(to alloc size)
  uint8_t nid;

  // iterate blocks
  for (b = heap->fblock; b; b = b->next)
  {
    // check if block has enough room
    if (b->size - (b->used * b->bsize) >= size)
    {
      bcnt = b->size / b->bsize;
      bneed = (size / b->bsize) * b->bsize < size ?
        size / b->bsize + 1 : size / b->bsize;
      bm = (uint8_t*)&b[1];

      for (x = (b->lfb + 1 >= bcnt ? 0 : b->lfb + 1);
//        x < b->lfb; // BUG in original implementation?
        x < bcnt;
        ++x)
      {
        // just wrap around
        if (x >= bcnt)
        {
          x = 0;
        }

        if (bm[x] == 0)
        {
          // count free blocks
          for (y = 0; bm[x + y] == 0 && y < bneed && (x + y) < bcnt; ++y);

          // we have enough, now allocate them
          if (y == bneed)
          {
            // find ID that does not match left or right
            nid = module_heap_get_nid(bm[x - 1], bm[x + y]);

            // allocate by setting id
            for (z = 0; z < y; ++z)
            {
              bm[x + z] = nid;
            }

            // optimization
            b->lfb = (x + bneed) - 2;

            // count used blocks NOT bytes
            b->used += y;

            return (void*)(x * b->bsize + (uintptr_t)&b[1]);
          }

          // x will be incremented by one ONCE more in our FOR loop
          x += (y - 1);
          continue;
        }
      }
    }
//    else // block has no room, go to next block
//    {
//      module_terminal_global_print_c_string("blk has no room\n");
//    }
  }
  return 0; // so called NULL ptr
}
// -------------------------------------------------------------------------- //
void module_heap_free(module_heap_heap_bm *heap, const void * const ptr)
{
  module_heap_heap_block_bm *b;
  uintptr_t ptroff;
  uint32_t bi, x;
  uint8_t *bm;
  uint8_t id;
  uint32_t max;

  for (b = heap->fblock; b; b = b->next)
  {
    if ((uintptr_t)ptr > (uintptr_t)b
      && (uintptr_t)ptr < (uintptr_t)b
      + sizeof(module_heap_heap_block_bm) + b->size)
    {
      // found block
      ptroff = (uintptr_t)ptr - (uintptr_t)&b[1];  // get offset to get block
      // block offset in BM
      bi = ptroff / b->bsize;
      // ..
      bm = (uint8_t*)&b[1];
      // clear allocation 
      id = bm[bi];
      // oddly.. GCC did not optimize this
      max = b->size / b->bsize;
      for (x = bi; bm[x] == id && x < max; ++x)
      {
        bm[x] = 0;
      }
      // update free block count
      b->used -= x - bi;
      return;
    }
  }
  // this error needs to be raised or reported somehow
  return;
}
// -------------------------------------------------------------------------- //
