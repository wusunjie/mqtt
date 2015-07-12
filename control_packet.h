/*
 * control_packet.h
 *
 *  Created on: 2015Äê7ÔÂ11ÈÕ
 *      Author: Administrator
 */

#ifndef CONTROL_PACKET_H_
#define CONTROL_PACKET_H_

#include "mqtt_types.h"

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


extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_DUP;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_0;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_1;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_2;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_RETAIN;
extern const mqtt_type_uint8 CONTROL_PACKET_COMMON_FLAGS_RESERVED;

extern const mqtt_type_uint8 CONNECT_RETURN_CODE_ACCEPT;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_WRONG_VERSION;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_WRONG_ID;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_INTERNAL;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_FAILED;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_AUTH;

struct connect_param {
	mqtt_type_uint8 level;
	mqtt_type_uint16 alive;
	struct cph_flag {
		unsigned int unf:1;
		unsigned int  pf:1;
		unsigned int  wr:1;
		unsigned int wqs:2;
		unsigned int  wf:1;
		unsigned int  cs:1;
		unsigned int    :1;
	} flags;
	struct mqtt_type_utf8 client_id;
	struct mqtt_type_utf8 w_topic;
	struct mqtt_type_utf8 w_mesg;
	struct mqtt_type_utf8 u_name;
	struct mqtt_type_utf8 pwd;
};

struct connack_param {
	unsigned int sp:1;
	mqtt_type_uint8 cr_code;
};

struct publish_param {
	mqtt_type_uint8 flags;
	struct mqtt_type_utf8 t_name;
	mqtt_type_uint16 packet_id;
	struct mqtt_type_utf8 payload;
};

struct parse_result {
	mqtt_type_uint8 type;
	mqtt_type_uint8 error;
	union {
		mqtt_type_uint16 packet_id;
		struct connect_param connect;
		struct connack_param connack;
		struct publish_param publish;
	} content;
};

extern mqtt_type_uint32 make_connect_packet(struct connect_param param, mqtt_type_uint8 **buf);

extern mqtt_type_uint8 make_connack_packet(struct connack_param param, mqtt_type_uint32 *buf);

extern mqtt_type_uint32 make_publish_packet(struct publish_param param, mqtt_type_uint8 **buf);

extern mqtt_type_uint8 make_puback_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf);

extern mqtt_type_uint8 make_pubrec_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf);

extern mqtt_type_uint8 make_pubrel_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf);

extern mqtt_type_uint8 make_pubcomp_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf);

extern mqtt_type_uint8 make_unsuback_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf);

extern mqtt_type_uint8 make_ping_req_packet(mqtt_type_uint16 *buf);

extern mqtt_type_uint8 make_ping_rsp_packet(mqtt_type_uint16 *buf);

extern mqtt_type_uint8 make_disconnect_packet(mqtt_type_uint16 *buf);

extern mqtt_type_uint32 parse_control_packet(mqtt_type_uint8 *buf, mqtt_type_uint32 len, struct parse_result *result);

#endif /* CONTROL_PACKET_H_ */
