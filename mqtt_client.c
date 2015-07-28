/*
 * mqtt_client.c
 *
 *  Created on: 2015Äê7ÔÂ18ÈÕ
 *      Author: Administrator
 */

#include "platform/inc/connection.h"

#include <stdint.h>
#include "control_packet.h"
#include "fifo.h"

struct session {
	int dummy;
};

struct client {
	int fd;
	struct fifo *read_buf;
	struct fifo *write_buf;
	struct utf8 id;
	struct connect_param param;
	struct session *session;
	unsigned char connected;
	unsigned char status;
};

static struct client clients[10];

static void on_connected(int fd);
static void on_disconnected(int fd);
static void on_readable(int fd);
static void on_error(int fd, int code);

static struct session *session_create(void);
static void session_destory(struct session *s);

static struct session *session_create(void)
{
	return 0;
}

static void session_destory(struct session *s)
{
}

static void on_connected(int fd)
{
	unsigned int i;
	for (i = 0; i < 10; i++) {
		if (clients[i].fd == fd) {
			if (!clients[i].connected) {
				clients[i].connected = 1;
				if (clients[i].session) {
					if (clients[i].param.flags.cs) {
						session_destory(clients[i].session);
						clients[i].session = session_create();
						clients[i].status = 0;
					}
				} else {
					uint8_t *buf = 0;
					uint32_t len;
					clients[i].session = session_create();
					len = make_connect_packet(clients[i].param, &buf);
					fifo_write(clients[i].write_buf, buf, len);
					clients[i].status = 0;
					free(buf);
				}
			}
			break;
		}
	}
}

static void on_disconnected(int fd)
{
	unsigned int i;
	for (i = 0; i < 10; i++) {
		if (clients[i].fd == fd) {
			clients[i].connected = 0;
			clients[i].status = 0;
			fifo_destory(clients[i].read_buf);
			fifo_destory(clients[i].write_buf);
			break;
		}
	}
}

static void on_readable(int fd)
{
	unsigned int i;
	for (i = 0; i < 10; i++) {
		if (clients[i].fd == fd) {
			if (clients[i].connected) {
				unsigned int len = fifo_available(clients[i].read_buf);
				unsigned char *buf = (unsigned char *)malloc(len);
				unsigned int l = fifo_read(clients[i].read_buf, buf, len);
				struct parse_result result;
				fifo_flush(clients[i].read_buf, parse_control_packet(buf, l, &result));
				switch (result.type) {
				case CONTROL_PACKET_TYPE_CONNACK:
				{
					if (0 == clients[i].status) {
						if (CONTROL_PACKET_ERROR_NONE == result.error) {
							if (result.content.connack.cr_code) {
								/* error. */
							} else {
								if (clients[i].param.flags.cs) {
									if (result.content.connack.sp) {
										/* error. */
									} else {
										/* clean session started. */
										clients[i].status = 1;
									}
								} else {
									if (result.content.connack.sp) {
										/* session reused. */
									} else {
										/* clean session started. */
									}
									clients[i].status = 1;
								}
							}
							if (1 == clients[i].status) {
								if (clients[i].param.alive) {
									/* start alive timer. */
								}
							}
						} else {
							/* packet format error. */
						}
					} else {
						/* sequence error. */
					}
				}
					break;
				default:
					break;
				}
				free(buf);
			} else {
				/* internal error. */
			}
			break;
		}
	}
}

static void on_error(int fd, int code)
{
	unsigned int i;
	for (i = 0; i < 10; i++) {
		if (clients[i].fd == fd) {
			clients[i].connected = 0;
			clients[i].status = 0;
			fifo_destory(clients[i].read_buf);
			fifo_destory(clients[i].write_buf);
			break;
		}
	}
}

struct conn_if conn_callback = {
		on_connected,
		on_disconnected,
		on_readable,
		on_error
};
