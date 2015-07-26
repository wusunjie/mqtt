/*
 * system.h
 *
 *  Created on: 2015Äê7ÔÂ26ÈÕ
 *      Author: Administrator
 */

#ifndef PLATFORM_INC_SYSTEM_H_
#define PLATFORM_INC_SYSTEM_H_

typedef void *(*system_task_entry)(void *args);

extern void system_start(void);

extern void system_task_create(system_task_entry entry);

extern void system_task_destory(void);

extern void system_timer_task_create(system_task_entry entry, long time);

extern void system_timer_task_destory(void);

extern void system_flag_create(void);

extern void system_flag_wait_clr(void);

extern void system_flag_set(void);

extern void system_flag_destory(void);

#endif /* PLATFORM_INC_SYSTEM_H_ */
