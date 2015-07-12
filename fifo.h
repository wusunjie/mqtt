/*
 * fifo.h
 *
 *  Created on: 2015Äê7ÔÂ13ÈÕ
 *      Author: Administrator
 */

#ifndef FIFO_H_
#define FIFO_H_

struct fifo;

extern struct fifo *fifo_create(unsigned int size);

extern void fifo_destory(struct fifo *f);

extern int fifo_read(struct fifo *f, unsigned char *buf, unsigned int len);

extern int fifo_write(struct fifo *f, unsigned char *buf, unsigned int len);

extern unsigned int fifo_flush(struct fifo *f, unsigned int len);

extern unsigned int fifo_available(struct fifo *f);

#endif /* FIFO_H_ */
