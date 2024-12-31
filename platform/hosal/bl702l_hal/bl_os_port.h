#ifndef __BL_OS_PORT_H__
#define __BL_OS_PORT_H__


#include <stdint.h>
#include <stdio.h>
#include <string.h>


void *bl_os_timer_create(const char *name, uint32_t period, void *handler, int repeat);
int bl_os_timer_start(void *xTimer);
int bl_os_timer_stop(void *xTimer);
int bl_os_timer_change_period(void *xTimer, uint32_t period);
int bl_os_timer_reset(void *xTimer);
int bl_os_timer_delete(void *xTimer);

void *bl_os_semphr_create(void);
int bl_os_semphr_take(void *xSemaphore, uint32_t timeout);
int bl_os_semphr_give(void *xSemaphore);
int bl_os_semphr_delete(void *xSemaphore);

void *bl_os_event_group_create(void);
uint32_t bl_os_event_group_wait_bits(void *xEventGroup, uint32_t bits, uint32_t timeout);
int bl_os_event_group_clear_bits(void *xEventGroup, uint32_t bits);
int bl_os_event_group_set_bits(void *xEventGroup, uint32_t bits);
int bl_os_event_group_delete(void *xEventGroup);


#endif
