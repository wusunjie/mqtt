/*
 * control_packet.c
 *
 *  Created on: 2015Äê7ÔÂ11ÈÕ
 *      Author: Administrator
 */

#include <string.h>
#include <stdlib.h>

#include "mqtt_types.h"
#include "control_packet.h"

extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_DUP         = 0x08;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_0 = 0x00;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_1 = 0x02;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_2 = 0x04;
extern const mqtt_type_uint8 CONTROL_PACKET_PUBLISH_FLAGS_RETAIN      = 0x01;
extern const mqtt_type_uint8 CONTROL_PACKET_COMMON_FLAGS_RESERVED     = 0x01;

extern const mqtt_type_uint8 CONNECT_RETURN_CODE_ACCEPT        = 0;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_WRONG_VERSION = 1;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_WRONG_ID      = 2;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_INTERNAL      = 3;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_FAILED        = 4;
extern const mqtt_type_uint8 CONNECT_RETURN_CODE_AUTH          = 5;

struct control_packet_header {
	unsigned int type:4;
	unsigned int flag:4;
};

struct connect_packet_header {
	mqtt_type_uint8 pname[6];
	mqtt_type_uint8 level;
	struct cph_flag flags;
	mqtt_type_uint16 alive;
};

struct connack_packet_header {
	struct {
		unsigned int   :7;
		unsigned int sp:1;
	} ca_flag;
	mqtt_type_uint8 cr_code;
};

static mqtt_type_uint8 encode_remain_len(mqtt_type_uint32 len, mqtt_type_uint32 *code);
static mqtt_type_uint8 decode_remain_len(mqtt_type_uint32 *code, mqtt_type_uint32 *len);
static mqtt_type_uint32 calc_connect_payload_len(struct connect_param param);
static mqtt_type_uint32 calc_publish_payload_len(struct publish_param param);
static mqtt_type_uint8 make_common_packet(mqtt_type_uint8 type, mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf);
static mqtt_type_uint8 make_empty_packet(mqtt_type_uint8 type, mqtt_type_uint16 *buf);
static void make_connect_packet_name(mqtt_type_uint8 *header);
static mqtt_type_uint8 check_connect_packet_name(mqtt_type_uint8 *header);

static void parse_connect_packet(mqtt_type_uint8 *buf, mqtt_type_uint32 len, mqtt_type_uint8 flag, struct parse_result *result);
static void parse_publish_packet(mqtt_type_uint8 *buf, mqtt_type_uint32 len, mqtt_type_uint8 flag,  struct parse_result *result);

static mqtt_type_uint8 packet_fixed_flag_table[16] = {
		0, 0, 0, 0, 0, 0, 2, 0,
		2, 0, 2, 0, 0, 0, 0, 0
};

mqtt_type_uint8 encode_remain_len(mqtt_type_uint32 len, mqtt_type_uint32 *code)
{
	mqtt_type_uint8 ret;
	mqtt_type_uint8 *buf = (mqtt_type_uint8 *)code;
	for (ret = 0; len > 0; ret++) {
		buf[ret] = len % 128;
		if (len /= 128) {
			buf[ret] |= 128;
		} else {
			return ret;
		}
	}
	return ret;
}

mqtt_type_uint8 decode_remain_len(mqtt_type_uint32 *code, mqtt_type_uint32 *len)
{
	mqtt_type_uint32 mplr = 1;
	mqtt_type_uint32 value = 0;
	mqtt_type_uint8 i;
	mqtt_type_uint8 *buf = (mqtt_type_uint8 *)code;
	for (i = 0; 0 != buf[i] & 128; i++) {
		value += (buf[i] & 127) * mplr;
		mplr *= 128;
		if (mplr > 128 * 128 * 128) {
			return 0xff;
		}
	}
	*len = value;
	return i + 1;
}

void make_connect_packet_name(mqtt_type_uint8 *header)
{
	mqtt_type_uint8 name[6] = {0, 4, 'M', 'Q', 'T', 'T'};
	memcpy(header, name, 6);
}

mqtt_type_uint8 check_connect_packet_name(mqtt_type_uint8 *header)
{
	mqtt_type_uint8 name[6] = {0, 4, 'M', 'Q', 'T', 'T'};
	return !!memcmp(header, name, 6);
}

mqtt_type_uint32 calc_connect_payload_len(struct connect_param param)
{
	mqtt_type_uint32 len = 0;
	len += param.client_id.len + 2;
	if (param.flags.wf) {
		len += param.w_topic.len + 2;
		len += param.w_mesg.len + 2;
	}
	if (param.flags.unf) {
		len += param.u_name.len + 2;
	}
	if (param.flags.pf) {
		len += param.pwd.len + 2;
	}
	return len;
}

mqtt_type_uint32 calc_publish_payload_len(struct publish_param param)
{
	mqtt_type_uint32 len = 0;
	len += param.t_name.len + 4;
	len += param.payload.len + 2;
	return len;
}

mqtt_type_uint8 make_common_packet(mqtt_type_uint8 type, mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf)
{
	struct control_packet_header *fixed;
	mqtt_type_uint8 *buffer;
	buffer = (mqtt_type_uint8 *)buf;
	fixed = (struct control_packet_header *)buffer;
	fixed->type = type;
	fixed->flag = packet_fixed_flag_table[type];
	*(buffer + 1) = 2;
	*(buffer + 2) |= packet_id >> 8;
	*(buffer + 3) |= packet_id & 0xff;
	return 4;
}

mqtt_type_uint8 make_empty_packet(mqtt_type_uint8 type, mqtt_type_uint16 *buf)
{
	struct control_packet_header *fixed;
	mqtt_type_uint8 *buffer;
	buffer = (mqtt_type_uint8 *)buf;
	fixed = (struct control_packet_header *)buffer;
	fixed->type = type;
	fixed->flag = packet_fixed_flag_table[type];
	*(buffer + 1) = 0;
	return 2;
}

mqtt_type_uint32 make_connect_packet(struct connect_param param, mqtt_type_uint8 **buf)
{
	mqtt_type_uint8 len;
	mqtt_type_uint32 l;
	mqtt_type_uint8 *buffer;
	struct control_packet_header *fixed;
	struct connect_packet_header *header;
	mqtt_type_uint8 *payload;
	mqtt_type_uint32 remain = calc_connect_payload_len(param);
	len = encode_remain_len(remain, &l);
	buffer = (mqtt_type_uint8 *)calloc(1 + len + remain, 1);
	fixed = (struct control_packet_header *)buffer;
	fixed->type = CONTROL_PACKET_TYPE_CONNECT;
	fixed->flag = packet_fixed_flag_table[fixed->type];
	memcpy(buffer + 1, &l, len);
	header = (struct connect_packet_header *)(buffer + 1 + len);
	make_connect_packet_name(header->pname);
	header->level = param.level;
	header->flags = param.flags;
	header->alive = param.alive;
	payload = (mqtt_type_uint8 *)header + sizeof(*header);
	memcpy(payload, &param.client_id.len, 2);
	payload += 2;
	memcpy(payload, param.client_id.data, param.client_id.len);
	payload += param.client_id.len;
	memcpy(payload, &param.w_topic.len, 2);
	payload += 2;
	memcpy(payload, param.w_topic.data, param.w_mesg.len);
	payload += param.w_mesg.len;
	memcpy(payload, &param.u_name.len, 2);
	payload += 2;
	memcpy(payload, param.u_name.data, param.u_name.len);
	payload += param.u_name.len;
	memcpy(payload, &param.pwd.len, 2);
	payload += 2;
	memcpy(payload, param.pwd.data, param.pwd.len);
	*buf = buffer;
	return 1 + len + remain;
}

mqtt_type_uint8 make_connack_packet(struct connack_param param, mqtt_type_uint32 *buf)
{
	struct control_packet_header *fixed;
	struct connack_packet_header *header;
	mqtt_type_uint8 *buffer = (mqtt_type_uint8 *)&buf;
	fixed = (struct control_packet_header *)buffer;
	fixed->type = CONTROL_PACKET_TYPE_CONNACK;
	fixed->flag = packet_fixed_flag_table[fixed->type];
	*(buffer + 1) = 2;
	header = (struct connack_packet_header *)(buffer + 2);
	header->ca_flag.sp = param.sp;
	header->cr_code = param.cr_code;
	return 4;
}

mqtt_type_uint32 make_publish_packet(struct publish_param param, mqtt_type_uint8 **buf)
{
	mqtt_type_uint8 len;
	mqtt_type_uint32 l;
	mqtt_type_uint8 *buffer;
	struct control_packet_header *fixed;
	mqtt_type_uint8 *cur;
	mqtt_type_uint32 remain = calc_publish_payload_len(param);
	len = encode_remain_len(remain, &l);
	buffer = (mqtt_type_uint8 *)calloc(1 + len + remain, 1);
	fixed = (struct control_packet_header *)buffer;
	fixed->type = CONTROL_PACKET_TYPE_PUBLISH;
	fixed->flag = param.flags;
	memcpy(buffer + 1, &l, len);
	cur = buffer + len + 1;
	memcpy(cur, &param.t_name.len, 2);
	cur += 2;
	memcpy(cur, param.t_name.data, param.t_name.len);
	cur += param.t_name.len;
	memcpy(cur, &param.packet_id, 2);
	cur += 2;
	memcpy(cur, &param.payload.len, 2);
	cur += 2;
	memcpy(cur, param.payload.data, param.payload.len);
	*buf = buffer;
	return 1 + len + remain;
}

mqtt_type_uint8 make_puback_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBACK, packet_id, buf);
}

mqtt_type_uint8 make_pubrec_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBREC, packet_id, buf);
}

mqtt_type_uint8 make_pubrel_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBREL, packet_id, buf);
}

mqtt_type_uint8 make_pubcomp_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBCOMP, packet_id, buf);
}

mqtt_type_uint8 make_unsuback_packet(mqtt_type_uint16 packet_id, mqtt_type_uint32 *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_UNSUBACK, packet_id, buf);
}

mqtt_type_uint8 make_ping_req_packet(mqtt_type_uint16 *buf)
{
	return make_empty_packet(CONTROL_PACKET_TYPE_PINGREQ, buf);
}

mqtt_type_uint8 make_ping_rsp_packet(mqtt_type_uint16 *buf)
{
	return make_empty_packet(CONTROL_PACKET_TYPE_PINGRESP, buf);
}

mqtt_type_uint8 make_disconnect_packet(mqtt_type_uint16 *buf)
{
	return make_empty_packet(CONTROL_PACKET_TYPE_DISCONNECT, buf);
}

mqtt_type_uint32 parse_control_packet(mqtt_type_uint8 *buf, mqtt_type_uint32 len, struct parse_result *result)
{
	mqtt_type_uint8 *cur = buf;
	mqtt_type_uint32 remain = 0;
	mqtt_type_uint8 l = 0;
	struct control_packet_header *fixed = (struct control_packet_header *)buf;
	if (len < sizeof(*fixed)) {
		return 0;
	}
	l = decode_remain_len((mqtt_type_uint32 *)(buf + 1), &remain);
	if (0xff == l) {
		return 0;
	}
	if (remain + l + 1 > len) {
		return 0;
	}
	result->type = fixed->type;
	switch (fixed->type) {
		case CONTROL_PACKET_TYPE_CONNECT:
			parse_connect_packet(buf + 1 + l, remain, fixed->flag, result);
			break;
		case CONTROL_PACKET_TYPE_CONNACK:
		{
			struct connack_packet_header *header = (struct connack_packet_header *)buf + 1 + l;
			result->content.connack.sp = header->ca_flag.sp;
			result->content.connack.cr_code = header->cr_code;
		}
			break;
		case CONTROL_PACKET_TYPE_PUBLISH:
			parse_publish_packet(buf + 1 + l, remain, fixed->flag, result);
			break;
		case CONTROL_PACKET_TYPE_PUBACK:
			result->content.packet_id = ((mqtt_type_uint16)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PUBREC:
			result->content.packet_id = ((mqtt_type_uint16)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PUBREL:
			result->content.packet_id = ((mqtt_type_uint16)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PUBCOMP:
			result->content.packet_id = ((mqtt_type_uint16)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_SUBSCRIBE:
			/* TO BE IMPLEMENT. */
			break;
		case CONTROL_PACKET_TYPE_SUBACK:
			/* TO BE IMPLEMENT. */
			break;
		case CONTROL_PACKET_TYPE_UNSUBSCRIBE:
			/* TO BE IMPLEMENT. */
			break;
		case CONTROL_PACKET_TYPE_UNSUBACK:
			result->content.packet_id = ((mqtt_type_uint16)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PINGREQ:
			break;
		case CONTROL_PACKET_TYPE_PINGRESP:
			break;
		case CONTROL_PACKET_TYPE_DISCONNECT:
			break;
		default:
			break;
	}
	return remain + l + 1;
}

void parse_connect_packet(mqtt_type_uint8 *buf, mqtt_type_uint32 len, mqtt_type_uint8 flag, struct parse_result *result)
{
	struct connect_packet_header *header = (struct connect_packet_header *)buf;
	if (check_connect_packet_name(header->pname)) {
		/* wrong protocol name. */
		result->error = 0;
		return;
	}
	result->content.connect.level = header->level;
	result->content.connect.flags = header->flags;
	result->content.connect.alive = header->alive;
	buf += sizeof(*header);
	result->content.connect.client_id.len = *(mqtt_type_uint16 *)buf;
	memcpy(result->content.connect.client_id.data, buf + 2, *(mqtt_type_uint16 *)buf);
	buf += 2 + *(mqtt_type_uint16 *)buf;
	if (header->flags.wf) {
		result->content.connect.w_topic.len = *(mqtt_type_uint16 *)buf;
		memcpy(result->content.connect.w_topic.data, buf + 2, *(mqtt_type_uint16 *)buf);
		buf += 2 + *(mqtt_type_uint16 *)buf;
		result->content.connect.w_mesg.len = *(mqtt_type_uint16 *)buf;
		memcpy(result->content.connect.w_mesg.data, buf + 2, *(mqtt_type_uint16 *)buf);
		buf += 2 + *(mqtt_type_uint16 *)buf;
	}
	if (header->flags.unf) {
		result->content.connect.u_name.len = *(mqtt_type_uint16 *)buf;
		memcpy(result->content.connect.u_name.data, buf + 2, *(mqtt_type_uint16 *)buf);
		buf += 2 + *(mqtt_type_uint16 *)buf;
	}
	if (header->flags.pf) {
		result->content.connect.pwd.len = *(mqtt_type_uint16 *)buf;
		memcpy(result->content.connect.pwd.data, buf + 2, *(mqtt_type_uint16 *)buf);
		buf += 2 + *(mqtt_type_uint16 *)buf;
	}
}

void parse_publish_packet(mqtt_type_uint8 *buf, mqtt_type_uint32 len,  mqtt_type_uint8 flag, struct parse_result *result)
{
	/* TO BE IMPLEMENT. */
}
