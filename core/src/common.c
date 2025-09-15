#include <stdio.h>
#include "common.h"
#include "k_task.h"

/**
 * @brief Call SVC to init kernel
 * 
 * @retval None
 */
void osKernelInit(void) {
  __asm("SVC #0");
}

/**
 * @brief Call SVC to create task
 * 
 * @retval RTX_OK on success, RTX_OFF on failure
 */
int osCreateTask(TCB *task) {
  if (task == NULL) {
    return RTX_ERR;
  }

  int ret;
  __asm(
    "SVC #1\n"
    // Copy r0 to return var
    "MOV %[out], r0\n"
    : [out] "=r" (ret)
  );
  return ret;
}

/**
 * @brief Call SVC to start kernel
 * 
 * @retval RTX_OK on success, RTX_OFF on failure
 */
int osKernelStart(void) {
  int ret;
  __asm(
    "SVC #2\n"
    "MOV %[out], r0\n"
    : [out] "=r" (ret)
  );
  // This shouldn't return if kernel started succesfully
  return ret;
}

/**
 * @brief Call SVC to yield current task
 * 
 * @retval None
 */
void osYield(void) {
  __asm("SVC #3");
}

/**
 * @brief Call SVC to get task info
 * 
 * @retval RTX_OK if TID exists, RTX_ERR on failure
 */

int osTaskInfo(task_t tid, TCB* task_copy) {
  if (tid >= MAX_TASKS || task_copy == NULL) {
    return RTX_ERR;
  }

  int ret;
  __asm(
    "SVC #4\n"
    // Copy r0 to return var
    "MOV %[out], r0\n"
    : [out] "=r" (ret)
    : "r" (tid), "r" (task_copy)
  );
  return ret;
}

/**
 * @brief Call SVC to get current task's TID
 * 
 * @retval TID of current task on success, 0 on failure
 */

task_t osGetTID(void) {
  int ret;
  __asm(
    "SVC #5\n"
    "MOV %[out], r0\n"
    : [out] "=r" (ret)
  );
  return ret;
} 

/**
 * @brief Call SVC to exit current task
 * 
 * @retval None on success, RTX_ERR on failure
 */
int osTaskExit(void) {
  int ret;
  __asm(
    "SVC #6\n"
    "MOV %[out], r0\n"
    : [out] "=r" (ret)
  );
  // This shouldn't return if the task exited properly
  return ret;
}

/**
 * @brief Call SVC to initialize memory
 * 
 * @retval RTX_OK on success, RTX_ERR on failure
 */
int k_mem_init() {
    int ret;
    __asm(
        "SVC #7\n"
        // Copy r0 to return var
        "MOV %[out], r0\n"
        : [out] "=r" (ret)
    );
    return ret;
}

/**
 * @brief Call SVC to allocate memory
 * 
 * @retval Pointer to allocated memory on success, NULL on failure
 */
void *k_mem_alloc(size_t size) {
    void *ptr;
    __asm(
        "SVC #8\n"
        // Copy r0 to return var
        "MOV %[out], r0\n"
        : [out] "=r" (ptr)
        : "r" (size)
    );
    return ptr;
}

/**
 * @brief Call SVC to deallocate memory
 * 
 * @retval RTX_OK on success or pointer is NULL, RTX_ERR on failure
 */
int k_mem_dealloc(void * ptr) {
  if (ptr == NULL) return RTX_OK;

  int ret;
  __asm(
      "SVC #9\n"
      // Copy r0 to return var
      "MOV %[out], r0\n"
      : [out] "=r" (ret)
  );
  return ret;
}

/**
 * @brief Call SVC to count external fragmentation
 * 
 * @retval Number of free blocks smaller than size
 */
int k_mem_count_extfrag(size_t size) {
  int ret;
  __asm(
      "SVC #10\n"
      "MOV %[out], r0\n"
      : [out] "=r" (ret)
      : "r" (size)       // size goes into r0
  );
  return ret;
}

/**
 * @brief Call SVC to set deadline for a task
 * 
 * @retval RTX_OK on success, RTX_ERR on failure
 */
int osSetDeadline(int deadline, task_t TID) {
    int ret;
    __asm(
        "SVC #13\n"
        // Copy r0 to return var
        "MOV %[out], r0\n"
        : [out] "=r" (ret)
        : "r" (deadline), "r" (TID) // deadline and TID go into r0 and r1
    );
    return ret;
}
