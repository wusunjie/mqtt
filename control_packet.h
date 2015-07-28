/*
 * control_packet.h
 *
 *  Created on: 2015Äê7ÔÂ11ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_PACKET_H_
#define CONTROL_PACKET_H_

#include <stdint.h>
#include "utf8.h"

enum {
	CONTROL_PACKET_TYPE_RESERVED1   = 0,
	CONTROL_PACKET_TYPE_CONNECT     = 1,
	CONTROL_PACKET_TYPE_CONNACK     = 2,
	CONTROL_PACKET_TYPE_PUBLISH     = 3,
	CONTROL_PACKET_TYPE_PUBACK      = 4,
	CONTROL_PACKET_TYPE_PUBREC      = 5,
	CONTROL_PACKET_TYPE_PUBREL      = 6,
	CONTROL_PACKET_TYPE_PUBCOMP     = 7,
	CONTROL_PACKET_TYPE_SUBSCRIBE   = 8,
	CONTROL_PACKET_TYPE_SUBACK      = 9,
	CONTROL_PACKET_TYPE_UNSUBSCRIBE = 10,
	CONTROL_PACKET_TYPE_UNSUBACK    = 11,
	CONTROL_PACKET_TYPE_PINGREQ     = 12,
	CONTROL_PACKET_TYPE_PINGRESP    = 13,
	CONTROL_PACKET_TYPE_DISCONNECT  = 14,
	CONTROL_PACKET_TYPE_RESERVED2   = 15
};

enum {
	CONTROL_PACKET_ERROR_NONE = 0,
	CONTROL_PACKET_ERROR_INCOMPLETE = 1,
	CONTROL_PACKET_ERROR_WRONG_LEN = 2,
	CONTROL_PACKET_ERROR_INVALID_FLAG = 3,
	CONTROL_PACKET_ERROR_WRONG_NAME = 4,
	CONTROL_PACKET_ERROR_INVALID_RESERVE
};


extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_DUP;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_0;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_1;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_2;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_RETAIN;
extern const uint8_t CONTROL_PACKET_COMMON_FLAGS_RESERVED;

extern const uint8_t CONNECT_RETURN_CODE_ACCEPT;
extern const uint8_t CONNECT_RETURN_CODE_WRONG_VERSION;
extern const uint8_t CONNECT_RETURN_CODE_WRONG_ID;
extern const uint8_t CONNECT_RETURN_CODE_INTERNAL;
extern const uint8_t CONNECT_RETURN_CODE_FAILED;
extern const uint8_t CONNECT_RETURN_CODE_AUTH;

struct connect_param {
	uint8_t level;
	uint16_t alive;
	struct cph_flag {
		unsigned int    :1;
		unsigned int  cs:1;
		unsigned int  wf:1;
		unsigned int wqs:2;
		unsigned int  wr:1;
		unsigned int  pf:1;
		unsigned int unf:1;
	} flags;
	struct utf8 client_id;
	struct utf8 w_topic;
	struct utf8 w_mesg;
	struct utf8 u_name;
	struct utf8 pwd;
};

struct connack_param {
	unsigned int sp:1;
	uint8_t cr_code;
};

struct publish_param {
	uint8_t flags;
	struct utf8 t_name;
	uint16_t packet_id;
	struct utf8 payload;
};

struct parse_result {
	uint8_t type;
	uint8_t error;
	union {
		uint16_t packet_id;
		struct connect_param connect;
		struct connack_param connack;
		struct publish_param publish;
	} content;
};

extern uint32_t make_connect_packet(struct connect_param param, uint8_t **buf);

extern uint8_t make_connack_packet(struct connack_param param, uint32_t *buf);

extern uint32_t make_publish_packet(struct publish_param param, uint8_t **buf);

extern uint8_t make_puback_packet(uint16_t packet_id, uint32_t *buf);

extern uint8_t make_pubrec_packet(uint16_t packet_id, uint32_t *buf);

extern uint8_t make_pubrel_packet(uint16_t packet_id, uint32_t *buf);

extern uint8_t make_pubcomp_packet(uint16_t packet_id, uint32_t *buf);

extern uint8_t make_unsuback_packet(uint16_t packet_id, uint32_t *buf);

extern uint8_t make_ping_req_packet(uint16_t *buf);

extern uint8_t make_ping_rsp_packet(uint16_t *buf);

extern uint8_t make_disconnect_packet(uint16_t *buf);

extern uint32_t parse_control_packet(uint8_t *buf, uint32_t len, struct parse_result *result);

#endif /* CONTROL_PACKET_H_ */
