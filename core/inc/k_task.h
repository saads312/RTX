/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

#include "common.h"

#define DEFAULT_DEADLINE 5 //ms

void kernelInit(void);
int createTask(TCB *task);
int startKernel(void);
int taskInfo(task_t tid, TCB* task_copy);
task_t getTID(void);
void yield(void);
void scheduler(void);
int taskExit(void);
void change_task(void);

int setDeadline(int deadline, task_t TID);

#endif /* INC_K_TASK_H_ */
