/*
 * connection.h
 *
 *  Created on: 2015��7��13��
 *      Author: Administrator
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

struct conn_if {
	void (*connected)(int fd);
	void (*disconnected)(int fd);
	void (*readable)(int fd);
	void (*error)(int fd, int code);
};

struct fifo;

struct conn_buf {
	struct fifo *read;
	struct fifo *write;
};

extern void conn_initialize(struct conn_if *handler);

extern int conn_start(struct conn_buf buf, char *ip, unsigned short port);

extern void conn_stop(int fd);

extern void conn_finalize(void);

#endif /* CONNECTION_H_ */
