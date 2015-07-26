/*
 * remon.c
 *
 *  Created on: 2015Äê7ÔÂ26ÈÕ
 *      Author: Administrator
 */

typedef void (*remon_task_entry)(void);

static long tick_count = 0;

static remon_task_entry reset_init_task[] = {
0
};

static remon_task_entry short_task[] = {
0
};

static remon_task_entry middle_task[] = {
0
};

static remon_task_entry idle_task[] = {
0
};

static void *remon_task(void *args);
static void *remon_timer_task(void *args);
static void *remon_idle_task(void *args);

void remon_start(void)
{
	tick_count = 0;
	system_flag_create();
	system_timer_task_create(remon_timer_task, 2);
	system_task_create(remon_task);
	system_task_create(remon_idle_task);
	system_start();
}

void *remon_task(void *args)
{
	(void)args;
	unsigned int j;
	for (j = 0; j < sizeof(reset_init_task); j++) {
		reset_init_task[j]();
	}
	while (1) {
		if (tick_count < 4) {
			unsigned int i;
			for (i = 0; i < sizeof(short_task); i++) {
				short_task[i]();
			}
			tick_count++;
		} else {
			unsigned int i;
			for (i = 0; i < sizeof(middle_task); i++) {
				middle_task[i]();
			}
			tick_count = 0;
		}
		system_flag_wait_clr();
	}
	return 0;
}

void *remon_idle_task(void *args)
{
	(void)args;
	while (1) {
		unsigned int i;
		for (i = 0; i < sizeof(idle_task); i++) {
			idle_task[i]();
		}
	}
	return 0;
}

void *remon_timer_task(void *args)
{
	(void)args;
	system_flag_set();
	return 0;
}
