/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!

/* USER CODE BEGIN Includes */
#include "common.h"
#include "k_task.h"
#include "k_mem.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 1 */
void mem1(void *);
void mem2(void *);
void mem3(void *);
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
/* USER CODE END 2 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* MCU Configuration: Don't change this or the whole chip won't work!*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  HAL_SuspendTick();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* MCU Configuration is now complete. Start writing your code below this line */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("Hello, world!\r\n");

  osKernelInit();
  k_mem_init();
  TCB test_task;
  test_task.ptask = mem1;
  test_task.stack_size = THREAD_STACK_SIZE;
  test_task.state = READY;
  int ret = osCreateTask(&test_task);
  printf("Create test task returned: %i\r\n", ret);

  TCB test_task2;
  test_task2.ptask = mem2;
  test_task2.stack_size = THREAD_STACK_SIZE;
  test_task2.state = READY;
  ret = osCreateTask(&test_task2);
  printf("Create test task 2 returned: %i\r\n", ret);

  // TCB test_task3;
  // test_task3.ptask = mem3;
  // test_task3.stack_size = THREAD_STACK_SIZE;
  // test_task3.state = READY;
  // ret = osCreateTask(&test_task3);
  // printf("Create test task 3 returned: %i\r\n", ret);

  ret = osKernelStart();
  printf("Kernel start failed: %i\r\n", ret);

  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
void mem1(void *) {
  int counter1 = 0;
  while (1) {
    if (counter1%1000 == 0) printf("Counter1 = %i\r\n", counter1);
    if (counter1 == 1000000) {
      printf("Yielding!\r\n");
      osYield();
    }
    counter1++;
  }
}


void mem2(void *) {
  int counter2 = 0;
  while (1) {
    if (counter2%1000 == 0) printf("Counter2 = %i\r\n", counter2);
    if (counter2 == 1000000) {
      printf("Yielding!\r\n");
      osYield();
    }
    counter2++;
  }
}

void mem3(void *) {
  int counter3 = 0;
  while (1) {
    if (counter3%10000 == 0) {
      printf("Thread 3, counter3 = %u\r\n", counter3);
    }
    counter3++;
  }
}
/* USER CODE END 4 */

