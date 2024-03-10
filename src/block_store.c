#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)


struct block_store
{
   size_t id;
   size_t used_blocks;
   size_t free_blocks;
   size_t total_blocks;
   size_t bytes_read;
   size_t bytes_written;
   int32_t blocks[BLOCK_STORE_NUM_BLOCKS];
   bitmap_t* bitmap;
   bool success;
};

///
/// This creates a new BS device, ready to go
/// \return Pointer to a new block storage device, NULL on error
///
block_store_t *block_store_create()
{
    block_store_t *bs = (block_store_t *) calloc(1, sizeof(block_store_t)); // zero intialize block store

    if(bs){
        // create free block bitmap from overlay of starting block
        bs->bitmap = bitmap_overlay(BITMAP_SIZE_BITS, &bs->blocks[BITMAP_START_BLOCK]); 
        // requests to assign the starting block to the bit map
        if(bs->bitmap && block_store_request(bs, BITMAP_START_BLOCK) && block_store_request(bs, BITMAP_START_BLOCK + 1)){  
            return bs;
        }
    }
    free(bs);
    return NULL; // errored out
}

void block_store_destroy(block_store_t *const bs)
{
    if (bs != NULL) {
        free(bs->bitmap);
    }
    free(bs);
}

///
/// Searches for a free block, marks it as in use, and returns the block's id
/// \param bs BS device
/// \return Allocated block's id, SIZE_MAX on error
///
size_t block_store_allocate(block_store_t *const bs)
{
    if (bs && bs->bitmap){
        size_t index = bitmap_ffz(bs->bitmap);      // find the first zero in the bit map, ie the first unallocated block
        if (index != SIZE_MAX) {
            bitmap_set(bs->bitmap, index);          // set that zero to one, claiming that block as allocated
            return index;                           // return the index of the allocated block
        }
    }

    return SIZE_MAX; //errored out
}

/// marks a specific block as allocated in the bitmap. 
/// @param bs block storage device 
/// @param block_id id of the block
/// @return true if the block was successfully marked as allocated, false otherwise.
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    // checks that bs is not null and that block_id is within valid range
    if(bs != NULL && block_id <= BITMAP_SIZE_BITS) {
        if(bitmap_test(bs->bitmap, block_id)){ return false; }          // return false if the block is already allocated
        else {
            bitmap_set(bs->bitmap, block_id);                           // mark block as allocated
            if(bitmap_test(bs->bitmap, block_id)){ return true; }       // test that block was marked
            else { return false; }                                      // false if marking fails
        }
    }
    //return false if the pointer is null or if the block_id is invalid
    return false; 
}

///
/// Frees the specified block
/// \param bs BS device
/// \param block_id The block to free
///
void block_store_release(block_store_t *const bs, const size_t block_id)
{
    //  checks that bs is not NULL and  block_id is within the range of valid block indices
    if (bs != NULL && block_id <= BITMAP_SIZE_BITS) {
        bitmap_reset(bs->bitmap, block_id);     //clears the bit in the bitmap
    }
    return;
}

///
/// Counts the number of blocks marked as in use
/// \param bs BS device
/// \return Total blocks in use, SIZE_MAX on error
///
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    // Check for NULL pointer
    if (bs != NULL) {
        return bitmap_total_set(bs->bitmap);   //returns number of set bits in the bitmap
    }
    return SIZE_MAX;    //error
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    UNUSED(bs);
    return 0;
}

size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_NUM_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    UNUSED(bs);
    UNUSED(block_id);
    UNUSED(buffer);
    return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    UNUSED(bs);
    UNUSED(block_id);
    UNUSED(buffer);
    return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
    UNUSED(filename);
    return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    UNUSED(bs);
    UNUSED(filename);
    return 0;
}