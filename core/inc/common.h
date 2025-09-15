#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#define TID_NULL    0 //predefined Task ID for the NULL task
#define MAX_TASKS   16 //maximum number of tasks in the system
#define STACK_SIZE  0x200 //min. size of each task’s stack

#define DORMANT     0 //state of terminated task
#define READY       1 //state of task that can be scheduled but is not running
#define RUNNING     2 //state of running task
#define SLEEPING    3 //state of sleeping task

#define RTX_OK  0
#define RTX_ERR 1

#define MAIN_STACK_SIZE     0x400
#define THREAD_STACK_SIZE   0x400

typedef unsigned int U32;
typedef unsigned short U16;
typedef char U8;
typedef unsigned int task_t;


typedef struct task_control_block {
    void    (*ptask)(void* args);   //entry address
    U32     stack_high;             //starting address of stack (high address)
    task_t  tid;                    //task ID
    U8      state;                  //task's state
    U16     stack_size;             //stack size. Must be a multiple of 8
    U32     stackptr;               //stack top address
    U32     deadline;               //configured deadline (ms)
    U32     time_left;              //time left in to deadline (ms)
} TCB;

// Multithreading functions
void osKernelInit(void);
int osCreateTask(TCB *task);
int osKernelStart(void);
void osYield(void);
int osTaskInfo(task_t tid, TCB* task_copy);
task_t osGetTID(void);
int osTaskExit(void);

// pre-emptive multitasking functions
int osSetDeadline(int deadline, task_t TID);

#endif /* INC_COMMON_H_ */
