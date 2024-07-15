#ifndef __ROM_API_H__
#define __ROM_API_H__

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

extern StackType_t *pxIdleTaskStackBuffer;
extern uint32_t ulIdleTaskStackSize;
extern StackType_t *pxTimerTaskStackBuffer;
extern uint32_t ulTimerTaskStackSize;

extern void *(*rtos_memcpy_ptr)(void *dest, const void *src, size_t n);
extern void *(*rtos_memset_ptr)(void *s, int c, size_t n);
extern char *(*rtos_strcpy_ptr)(char *dest, const char *src);
extern size_t (*rtos_strlen_ptr)(const char *s);
extern int (*rtos_sprintf_ptr)(char *str, const char *format, ...);
extern int (*rtos_clz_ptr)(unsigned int x);

extern void (*exception_entry_ptr)(uint32_t mcause, uint32_t mepc, uint32_t mtval, uintptr_t *regs, uintptr_t *tasksp);
extern void (*interrupt_entry_ptr)(uint32_t mcause);
extern void (*check_trap_ptr)(uint32_t label_is_exception, uint32_t sp);

extern void (*vApplicationGetIdleTaskMemory)(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);
extern void (*vApplicationGetTimerTaskMemory)(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize);
extern void (*vApplicationIdleHook)(void);
extern void (*vApplicationMallocFailedHook)(void);
extern void (*vApplicationStackOverflowHook)(TaskHandle_t xTask, char *pcTaskName);
extern void (*vAssertCalled)(void);
extern void (*vApplicationTickHook)(void);
extern void (*vApplicationSleep)(TickType_t xExpectedIdleTime);
extern void (*vApplicationTimerPreHook)(void *pvTimerID, TimerCallbackFunction_t pxCallbackFunction);
extern void (*vApplicationTimerPostHook)(void *pvTimerID, TimerCallbackFunction_t pxCallbackFunction);

extern void freertos_risc_v_trap_handler(void);
extern StackType_t xISRStackTop;
extern uint64_t ullNextTime;
extern const uint64_t *pullNextTime;
extern size_t uxTimerIncrementsForOneTick;
extern volatile uint64_t * const pullMachineTimerCompareRegister;
extern BaseType_t TrapNetCounter;
extern const BaseType_t  *pTrapNetCounter;

#endif
