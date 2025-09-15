#include <stdio.h>
#include "k_mem.h"
#include "k_task.h"
#include "stm32f401xe.h"

extern U32 _img_end; // end of the image, defined in linker script
extern U32 _estack; // end of the stack, defined in linker script
extern U32 _Min_Stack_Size; // minimum stack size, defined in linker script

struct metaHeader *freelist_head = NULL; // the head of the freelist linked list
static int already_initialized = 0;

extern task_t running_task;

size_t max_heap_size;

/**
 * @brief Initialize memory management system
 * 
 * @retval RTX_OK on success, RTX_ERR on failure
 */
int mem_init() {
    if (already_initialized) return RTX_ERR;

    freelist_head = (metaHeader *)&_img_end; // start of free memory
     // initial full heap size
    freelist_head->size = ( (U32)(&_estack) - (U32)(&_Min_Stack_Size) ) - (U32)(&_img_end);
    freelist_head->next = NULL; // no next block
    freelist_head->tid = TID_NULL; // no owner
    freelist_head->is_allocated = 0; // initially free

    if (freelist_head->size < sizeof(metaHeader)) {
        return RTX_ERR; // not enough memory for metadata
    }

    already_initialized = 1;
    max_heap_size = freelist_head->size;
    
    return RTX_OK;
}

/**
 * @brief Allocate a block of memory requested by the user (DEBUG VERSION)
 *
 * @retval Pointer to allocated memory, or NULL if request fails
 */
void * mem_alloc(size_t size) {
    if (!already_initialized || size == 0) return NULL;

    size = (size + 3) & ~3; // align size to 4 bytes

    // traverse the freelist to find a suitable block
    metaHeader *current = freelist_head;
    metaHeader *prev = NULL;
    metaHeader* new_block = NULL;

    while (current != NULL) {
        if (!current->is_allocated && current->size >= size) {
            // a suitable block was found
            size_t remaining_size = current->size - size;

            if (remaining_size >= METADATA_SIZE + 4) {
                // if there's enough space for at least one more block (4B) + metadata
                new_block = (metaHeader*)((U8*)current + METADATA_SIZE + size);
                new_block->size = remaining_size - METADATA_SIZE;
                new_block->next = current->next;
                new_block->tid = TID_NULL;
                new_block->is_allocated = 0;

                current->size = size;
                current->next = new_block;
            }

            current->is_allocated = 1;
            current->tid = (U8)getTID();

            if (prev == NULL) {
                // head of freelist
                if (remaining_size >= METADATA_SIZE + 4) {
                    // split the block, so freelist head becomes the new free block
                    freelist_head = new_block;
                } else {
                    // we used the entire block, so freelist head becomes the next block
                    freelist_head = current->next;
                }
            } else {
                // not the head, link previous block to next
                if (remaining_size >= METADATA_SIZE + 4) {
                    // split the block, so prev points to the new free block
                    prev->next = new_block;
                } else {
                    // used the entire block, so previous points to current's next
                    prev->next = current->next;
                }
            }

            return (void*)((U8*)current + METADATA_SIZE);
        }
        prev = current;
        current = current->next;
    }

    return NULL;
}

int mem_dealloc(void *ptr) {
    // Check kernel memory structures are initialized
    if (!already_initialized) return RTX_ERR;

    // Do nothing is ptr is null
    if (ptr == NULL) return RTX_OK;

    // Check for valid ptr
    metaHeader *head = (metaHeader *) ((U8 *)ptr - METADATA_SIZE);
    if (head == NULL || head->is_allocated == 0 || (task_t)head->tid != running_task || head->size > max_heap_size) return RTX_ERR;

    // Clear metadata
    head->is_allocated = 0;
    head->tid = TID_NULL;

    // Add block to freelist
    metaHeader *tmp = freelist_head;
    metaHeader *prev = NULL;
    while (1) {
        if (tmp == NULL) {
            freelist_head = head;
            tmp = freelist_head;
            break;
        } else if (tmp->next == NULL) {
            if (tmp < head) {
                head->next = tmp->next;
                tmp->next = head;
            } else {
                head->next = tmp;
                if (prev == NULL) freelist_head = head;
                else prev->next = head;
                // Swap order of pointers for coalesce logic
                prev = tmp;
                tmp = head;
                head = prev;
            }
            break;
        } else if (tmp > head) {
            head->next = tmp;
            if (prev == NULL) freelist_head = head;
            else prev->next = head;
            // Swap order of pointers for coalesce logic
            prev = tmp;
            tmp = head;
            head = prev;
            break;
        } else if (tmp < head && tmp->next > head) {
            head->next = tmp->next;
            tmp->next = head;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }

    // Coalesce if needed
    if (((U8 *)tmp + tmp->size + METADATA_SIZE == (U8 *)head)
        && (head->next != NULL)
        && ((U8 *)head + head->size + METADATA_SIZE == (U8 *)head->next)) {
        // coalesce tmp, head, and next
        tmp->size += head->size + head->next->size + 2*METADATA_SIZE;
        tmp->next = head->next->next;
    } else if ((U8 *)tmp + tmp->size + METADATA_SIZE == (U8 *)head) {
        // coalesce tmp and head
        tmp->size += head->size + METADATA_SIZE;
        tmp->next = head->next;
    } else if ((head->next != NULL) && ((U8 *)head + head->size + METADATA_SIZE == (U8 *)head->next)) {
        // coalesce head and next
        head->size += head->next->size + METADATA_SIZE;
        head->next = head->next->next;
    }

    return RTX_OK;
}

int mem_count_extfrag(size_t size) {
    if (!already_initialized || freelist_head == NULL) {
        return 0; 
    }
    
    int count = 0;
    struct metaHeader *curr = freelist_head;

    while (curr != NULL) {
        if ((curr->size + METADATA_SIZE) < size) {
            count++;
        }
        curr = curr->next;
    }

    return count;
}