/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

#include "common.h"
#include "stddef.h"

typedef struct metaHeader {
    size_t size; // the size of the block
    struct metaHeader * next; // the next free block in memory
    U8 tid; // owner of the block in memory, null if free
    U8 is_allocated; // 1 if allocated, 0 if free
} metaHeader;
#define METADATA_SIZE sizeof(metaHeader)

extern struct metaHeader *freelist_head;

// User-side functions
int k_mem_init();
void * k_mem_alloc(size_t size);
int k_mem_dealloc(void * ptr);
int k_mem_count_extfrag(size_t size);

// Kernel-side functions
int mem_init();
void * mem_alloc(size_t size);
int mem_dealloc(void * ptr);
int mem_count_extfrag(size_t size);

#endif /* INC_K_MEM_H_ */
