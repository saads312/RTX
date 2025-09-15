#include <stdio.h>
#include "k_task.h"
#include "stm32f401xe.h"
#include "stm32f4xx_hal.h"
#include "k_mem.h"

TCB tasks[MAX_TASKS];
task_t running_task;
task_t selected_task;
U8 num_tasks;
U32 *MSP_INIT_VAL;
U8 kernel_init_done = 0;
volatile U32 stackptr;
volatile U32 *pendsv_reg;

/**
 * @brief Initialize kernel data structures
 * 
 * @retval None
 */
void kernelInit(void) {
    // First thread stack address
    MSP_INIT_VAL = *(U32**)0x0;

    // Initialize TCB array
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].tid = TID_NULL;
    }
    tasks[TID_NULL].stack_high = (U32)((char *)MSP_INIT_VAL - MAIN_STACK_SIZE);
    tasks[TID_NULL].stack_size = THREAD_STACK_SIZE;

    // Initialize global vars
    running_task = TID_NULL;
    selected_task = TID_NULL;
    num_tasks = 1;
    kernel_init_done = 1;
}

/**
 * @brief Copies data from tcb1 to tcb2
 * 
 * @retval None
 */
void copy_TCB(TCB *tcb1, TCB *tcb2) {
    tcb2->ptask = tcb1->ptask;
    tcb2->stack_high = tcb1->stack_high;
    tcb2->tid = tcb1->tid;
    tcb2->state = tcb1->state;
    tcb2->stack_size = tcb1->stack_size;
    tcb2->stackptr = tcb1->stackptr;
    tcb2->deadline = tcb1->deadline;
    tcb2->time_left = tcb1->time_left;
}

/**
 * @brief Register task with kernel
 * 
 * @retval RTX_OK on success, RTX_ERR on failure
 */
int createTask(TCB *task) {
    // Check for task slot and size
    if (num_tasks == MAX_TASKS || task->stack_size < STACK_SIZE) {
        return RTX_ERR;
    }

    // Find open slot for TCB in kernel array
    task_t tid = TID_NULL;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].tid == TID_NULL || tasks[i].state == DORMANT) {
            void *stack_high = mem_alloc(task->stack_size);
            if (stack_high != NULL) {
                tid = i;
                task->stack_high = (U32)stack_high;
                break;
            }
        }
    }
    
    // If no suitable slot found, return error
    if (tid == TID_NULL) {
        return RTX_ERR;
    }

    // Fill in TCB and copy to kernel array
    task->tid = tid;
    task->state = READY;
    task->deadline = task->time_left = DEFAULT_DEADLINE;
    copy_TCB(task, &tasks[tid]);

    // Setup new task's stack with dummy values
    U32 *tmp_stack;
    tmp_stack = (U32 *)tasks[tid].stack_high;
    *(--tmp_stack) = 1 << 24;
    *(--tmp_stack) = (U32)tasks[tid].ptask;
    for (int i = 0; i < 14; i++) {
        *(--tmp_stack) = 0xA;
    }

    tasks[tid].stackptr = (U32)tmp_stack;
    num_tasks++;

    // May cause issues if kernel hasn't been started?
    if (task->time_left < tasks[running_task].time_left)
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    return RTX_OK;
}

/**
 * @brief Execute task in first TCB
 * 
 * @retval RTX_OK on success, RTX_ERR on failure
 */
int startKernel(void) {
    // Check that kernel is initialized and no task is running
    if (!kernel_init_done || running_task != TID_NULL) {
        return RTX_ERR;
    }

    // Select first task
    tasks[running_task].state = RUNNING;
    stackptr = tasks[running_task].stack_high;
    __set_PSP(stackptr);
    scheduler();

    // Enable PendSV
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    HAL_ResumeTick();
    return RTX_OK;
}

/**
 * @brief Select next task available to run
 * 
 * @retval None
 */
void scheduler(void) {
    U32 shortest_deadline = UINT32_MAX;
    for (int i = 0; i < MAX_TASKS; i++) {
        // Select a non-null, non-terminated task with shortest deadline
        if (tasks[i].tid != TID_NULL && (tasks[i].state == READY || tasks[i].state == RUNNING)) {
            if ((tasks[i].time_left < shortest_deadline) || (tasks[i].time_left == shortest_deadline && i < selected_task)) {
                selected_task = i;
                shortest_deadline = tasks[i].time_left;
            }
        }
    }
}

/**
 * @brief Change tasks
 * 
 * @retval None
 */
void change_task(void) {
    // Update stack pointers
    stackptr = __get_PSP();
    tasks[running_task].stackptr = stackptr;
    stackptr = tasks[selected_task].stackptr;
    __set_PSP(stackptr);

    // Update states
    if (tasks[running_task].state != DORMANT) {
        tasks[running_task].state = READY;
    }
    tasks[selected_task].state = RUNNING;
    running_task = selected_task;
}

/**
 * @brief Get info on the current task and copy it into an empty TCB
 * 
 * @retval RTX_OK on success, RTX_ERR on failure
 */
int taskInfo(task_t tid, TCB* task_copy) {
    // Check if TID is valid and task_copy is not NULL
    if (tid >= MAX_TASKS || tasks[tid].tid == TID_NULL) {
        return RTX_ERR;
    }

    // Copy the TCB data to the provided task_copy
    copy_TCB(&tasks[tid], task_copy);
    return RTX_OK;
}

/**
 * @brief Get the TID of the currently running task
 * 
 * @retval TID of the currently running task, 0 if no task is running
 */
task_t getTID(void) {
    // If kernel is not initialized or no task is running
    if (!kernel_init_done || running_task == TID_NULL) {
        return TID_NULL;
    }
    return tasks[running_task].tid;
}

/**
 * @brief Save current task's context and switch to next available task
 * 
 * @retval None
 */
void yield(void) {
    // Reset deadline
    tasks[running_task].time_left = tasks[running_task].deadline;
    // Select next task
    scheduler();
    // Enable PendSV
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Exit currently running task
 * 
 * @retval RTX_ERR on failure
 */
int taskExit(void) {
    // If the kernel hasn’t been initialized or no user‐task is running
    if(!kernel_init_done || running_task == TID_NULL) {
        return RTX_ERR;
    }

    // Set running task to DORMANT and free the TID
    tasks[running_task].state = DORMANT;
    tasks[running_task].tid = TID_NULL; 
    num_tasks--;

    // Trigger PendSV to switch into the next task
    scheduler();
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    return RTX_OK;
}

/**
 * @brief Set deadline for any task
 *
 * @retval RTX_OK if deadline is strictly positive and task TID is ready, RTX_ERR otherwise
 */
int setDeadline(int deadline, task_t TID) {

    __disable_irq(); // Disable interrupts

    if (deadline <= 0 || TID >= MAX_TASKS || TID == running_task || tasks[TID].state != READY) {
        __enable_irq();
        return RTX_ERR;
    }

    tasks[TID].deadline = tasks[TID].time_left = deadline;

    // if task TID's deadline is less than currently running task, trigger context switch
    if (tasks[TID].deadline < tasks[running_task].deadline) {
        scheduler();
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    }

    __enable_irq();
    return RTX_OK;
}