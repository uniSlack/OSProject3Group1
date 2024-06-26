#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


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
   char blocks[BLOCK_STORE_NUM_BLOCKS][BLOCK_SIZE_BYTES];
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

/// Counts the number of blocks that are free
/// @param bs BS device
/// @return the number of free blocks, SIZE_MAX on error
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    //checks that the bs is not NULL
    if(bs != NULL){
        //gets the difference between the total number of blocks and the number of used blocks
        size_t dif = block_store_get_total_blocks(bs) - block_store_get_used_blocks(bs);    
        return dif;
    }
    return SIZE_MAX;    //error
}

size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_NUM_BLOCKS;
}

/// Reads data from the specified block and writes it to the designated buffer
/// \param bs BS device
/// \param block_id Source block id
/// \param buffer Data buffer to write to
/// \return Number of bytes read, 0 on error
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    if(bs && block_id < BLOCK_STORE_NUM_BLOCKS && buffer){
        if(memcpy(buffer, &(bs->blocks[block_id]), BLOCK_SIZE_BYTES)){
            return BLOCK_SIZE_BYTES;
        }
    }
    return 0;
}

/// Reads data from the specified buffer and writes it to the designated block
/// \param bs BS device
/// \param block_id Destination block id
/// \param buffer Data buffer to read from
/// \return Number of bytes written, 0 on error
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    if(bs && block_id < BLOCK_STORE_NUM_BLOCKS && buffer){
        block_store_request(bs, block_id);  // allocate on bitmap if not done before. Not sure what behaviour should be if already allocated, assuming intentional overwrite
        if(memcpy(&(bs->blocks[block_id]), buffer, BLOCK_SIZE_BYTES)){
            return BLOCK_SIZE_BYTES;
        }   
    }
    return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
    UNUSED(filename);
    return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    if(!bs || !filename){
        // Handle nulls
        return 0;
    }

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        // Handle file opening error
        return 0;
    }

    size_t bytes_written = 0;

    // Write block store data to the file
    ssize_t write_size = write(fd, bs->bitmap, sizeof(char) * BLOCK_STORE_NUM_BLOCKS * BLOCK_SIZE_BYTES);
    if (write_size != sizeof(char) * BLOCK_STORE_NUM_BLOCKS * BLOCK_SIZE_BYTES) {
        // Handle write error
        close(fd);
        return 0;
    }
    bytes_written += write_size;

    // Close the file
    close(fd);
    return bytes_written;
}