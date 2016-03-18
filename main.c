#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <inttypes.h>
#include <seu.h>

#define NVIC_PriorityGroup_4         ((uint32_t)0x300) /*!< 4 bits for pre-emption priority
                                                            0 bits for subpriority */
#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)

void test_FPU_test(void* p);

TaskHandle_t pvCreatedTask;

int main(void) {
    uint8_t ret;

    SCB->AIRCR = AIRCR_VECTKEY_MASK | NVIC_PriorityGroup_4;
    dprint("\n*** main: System Started @ %"PRIu32"hz ***\n\n", HSE_VALUE);

/*
 * Create a new task and add it to the list of tasks that are ready to run.
 */

    ret = xTaskCreate( test_FPU_test, "FPU", (uint16_t)(configMINIMAL_STACK_SIZE * 4), NULL, ((UBaseType_t)configTIMER_TASK_PRIORITY), NULL);
    if (ret == pdPASS) {
        dprint("FPU task created\nStarting Scheduler\n");
        vTaskStartScheduler();  // should never return
    } else {
        dprint("System Error!\n");
        // --TODO blink some LEDs to indicate fatal system error
    }

    dprint("System Crash!\n");
    for (;;);
}

#if configUSE_TICK_HOOK
extern uint64_t clock;

void vApplicationTickHook(void) {
    dprint("tock\n");
    clock++;
}
#endif

#if configUSE_MALLOC_FAILED_HOOK
/* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created.  It is also called by various parts of the
   demo application.  If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
void vApplicationMallocFailedHook(void) {
  taskDISABLE_INTERRUPTS();
  for(;;);
}
#endif

#if configUSE_IDLE_HOOK
/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
   task.  It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()).  If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
void vApplicationIdleHook(void) {
}
#endif

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
  (void) pcTaskName;
  (void) pxTask;
  /* Run time stack overflow checking is performed if
     configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     function is called if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  for(;;);
}

void test_FPU_test(void* p) {
    int x;
    for (;;) {
        dprint("FPU test.\n");

        for(x=0; x<2000000; x++) { }

        dprint("FPU delay.\n");
        vTaskDelay(1000);
    }
}

