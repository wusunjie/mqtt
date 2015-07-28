/*
 * control_packet.c
 *
 *  Created on: 2015Äê7ÔÂ11ÈÕ
 *      Author: Administrator
 */

#include <string.h>
#include <stdlib.h>

#include <stdint.h>
#include "control_packet.h"

extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_DUP         = 0x08;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_0 = 0x00;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_1 = 0x02;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_QOS_LEVEL_2 = 0x04;
extern const uint8_t CONTROL_PACKET_PUBLISH_FLAGS_RETAIN      = 0x01;
extern const uint8_t CONTROL_PACKET_COMMON_FLAGS_RESERVED     = 0x01;

extern const uint8_t CONNECT_RETURN_CODE_ACCEPT        = 0;
extern const uint8_t CONNECT_RETURN_CODE_WRONG_VERSION = 1;
extern const uint8_t CONNECT_RETURN_CODE_WRONG_ID      = 2;
extern const uint8_t CONNECT_RETURN_CODE_INTERNAL      = 3;
extern const uint8_t CONNECT_RETURN_CODE_FAILED        = 4;
extern const uint8_t CONNECT_RETURN_CODE_AUTH          = 5;

#pragma pack(push, 1)

struct control_packet_header {
	unsigned int flag:4;
	unsigned int type:4;
};

struct connect_packet_header {
	uint8_t pname[6];
	uint8_t level;
	struct cph_flag flags;
	uint16_t alive;
};

struct connack_packet_header {
	struct {
		unsigned int       sp:1;
		unsigned int reserved:7;
	} ca_flag;
	uint8_t cr_code;
};

#pragma pack(pop)

static uint8_t encode_remain_len(uint32_t len, uint32_t *code);
static uint8_t decode_remain_len(uint32_t *code, uint32_t *len);
static uint32_t calc_connect_payload_len(struct connect_param param);
static uint32_t calc_publish_payload_len(struct publish_param param);
static uint8_t make_common_packet(uint8_t type, uint16_t packet_id, uint32_t *buf);
static uint8_t make_empty_packet(uint8_t type, uint16_t *buf);
static void make_connect_packet_name(uint8_t *header);
static uint8_t check_connect_packet_name(uint8_t *header);

static void parse_connect_packet(uint8_t *buf, uint32_t len, struct parse_result *result);
static void parse_publish_packet(uint8_t *buf, uint32_t len, uint8_t flag,  struct parse_result *result);

static uint8_t packet_fixed_flag_table[16] = {
		0, 0, 0, 0, 0, 0, 2, 0,
		2, 0, 2, 0, 0, 0, 0, 0
};

uint8_t encode_remain_len(uint32_t len, uint32_t *code)
{
	uint8_t ret;
	uint8_t *buf = (uint8_t *)code;
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

uint8_t decode_remain_len(uint32_t *code, uint32_t *len)
{
	uint32_t mplr = 1;
	uint32_t value = 0;
	uint8_t i;
	uint8_t *buf = (uint8_t *)code;
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

void make_connect_packet_name(uint8_t *header)
{
	uint8_t name[6] = {0, 4, 'M', 'Q', 'T', 'T'};
	memcpy(header, name, 6);
}

uint8_t check_connect_packet_name(uint8_t *header)
{
	uint8_t name[6] = {0, 4, 'M', 'Q', 'T', 'T'};
	return !!memcmp(header, name, 6);
}

uint32_t calc_connect_payload_len(struct connect_param param)
{
	uint32_t len = 0;
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

uint32_t calc_publish_payload_len(struct publish_param param)
{
	uint32_t len = 0;
	len += param.t_name.len + 4;
	len += param.payload.len + 2;
	return len;
}

uint8_t make_common_packet(uint8_t type, uint16_t packet_id, uint32_t *buf)
{
	struct control_packet_header *fixed;
	uint8_t *buffer;
	buffer = (uint8_t *)buf;
	fixed = (struct control_packet_header *)buffer;
	fixed->type = type;
	fixed->flag = packet_fixed_flag_table[type];
	*(buffer + 1) = 2;
	*(buffer + 2) |= packet_id >> 8;
	*(buffer + 3) |= packet_id & 0xff;
	return 4;
}

uint8_t make_empty_packet(uint8_t type, uint16_t *buf)
{
	struct control_packet_header *fixed;
	uint8_t *buffer;
	buffer = (uint8_t *)buf;
	fixed = (struct control_packet_header *)buffer;
	fixed->type = type;
	fixed->flag = packet_fixed_flag_table[type];
	*(buffer + 1) = 0;
	return 2;
}

uint32_t make_connect_packet(struct connect_param param, uint8_t **buf)
{
	uint8_t len;
	uint32_t l;
	uint8_t *buffer;
	struct control_packet_header *fixed;
	struct connect_packet_header *header;
	uint8_t *payload;
	uint32_t remain = calc_connect_payload_len(param);
	len = encode_remain_len(remain, &l);
	buffer = (uint8_t *)calloc(1 + len + remain, 1);
	fixed = (struct control_packet_header *)buffer;
	fixed->type = CONTROL_PACKET_TYPE_CONNECT;
	fixed->flag = packet_fixed_flag_table[fixed->type];
	memcpy(buffer + 1, &l, len);
	header = (struct connect_packet_header *)(buffer + 1 + len);
	make_connect_packet_name(header->pname);
	header->level = param.level;
	header->flags = param.flags;
	header->alive = param.alive;
	payload = (uint8_t *)header + sizeof(*header);
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

uint8_t make_connack_packet(struct connack_param param, uint32_t *buf)
{
	struct control_packet_header *fixed;
	struct connack_packet_header *header;
	uint8_t *buffer = (uint8_t *)&buf;
	fixed = (struct control_packet_header *)buffer;
	fixed->type = CONTROL_PACKET_TYPE_CONNACK;
	fixed->flag = packet_fixed_flag_table[fixed->type];
	*(buffer + 1) = 2;
	header = (struct connack_packet_header *)(buffer + 2);
	header->ca_flag.sp = param.sp;
	header->cr_code = param.cr_code;
	return 4;
}

uint32_t make_publish_packet(struct publish_param param, uint8_t **buf)
{
	uint8_t len;
	uint32_t l;
	uint8_t *buffer;
	struct control_packet_header *fixed;
	uint8_t *cur;
	uint32_t remain = calc_publish_payload_len(param);
	len = encode_remain_len(remain, &l);
	buffer = (uint8_t *)calloc(1 + len + remain, 1);
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

uint8_t make_puback_packet(uint16_t packet_id, uint32_t *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBACK, packet_id, buf);
}

uint8_t make_pubrec_packet(uint16_t packet_id, uint32_t *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBREC, packet_id, buf);
}

uint8_t make_pubrel_packet(uint16_t packet_id, uint32_t *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBREL, packet_id, buf);
}

uint8_t make_pubcomp_packet(uint16_t packet_id, uint32_t *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_PUBCOMP, packet_id, buf);
}

uint8_t make_unsuback_packet(uint16_t packet_id, uint32_t *buf)
{
	return make_common_packet(CONTROL_PACKET_TYPE_UNSUBACK, packet_id, buf);
}

uint8_t make_ping_req_packet(uint16_t *buf)
{
	return make_empty_packet(CONTROL_PACKET_TYPE_PINGREQ, buf);
}

uint8_t make_ping_rsp_packet(uint16_t *buf)
{
	return make_empty_packet(CONTROL_PACKET_TYPE_PINGRESP, buf);
}

uint8_t make_disconnect_packet(uint16_t *buf)
{
	return make_empty_packet(CONTROL_PACKET_TYPE_DISCONNECT, buf);
}

uint32_t parse_control_packet(uint8_t *buf, uint32_t len, struct parse_result *result)
{
	uint8_t *cur = buf;
	uint32_t remain = 0;
	uint8_t l = 0;
	struct control_packet_header *fixed = (struct control_packet_header *)buf;
	result->error = CONTROL_PACKET_ERROR_NONE;
	if (len < sizeof(*fixed)) {
		result->error = CONTROL_PACKET_ERROR_INCOMPLETE;
		return 0;
	}
	l = decode_remain_len((uint32_t *)(buf + 1), &remain);
	if (0xff == l) {
		result->error = CONTROL_PACKET_ERROR_WRONG_LEN;
		return 0;
	}
	if (remain + l + 1 > len) {
		result->error = CONTROL_PACKET_ERROR_INCOMPLETE;
		return 0;
	}
	if (fixed->flag == packet_fixed_flag_table[fixed->type]) {
		result->error = CONTROL_PACKET_ERROR_INVALID_FLAG;
		return remain + l + 1;
	}
	result->type = fixed->type;
	switch (fixed->type) {
		case CONTROL_PACKET_TYPE_CONNECT:
			parse_connect_packet(buf + 1 + l, remain, result);
			break;
		case CONTROL_PACKET_TYPE_CONNACK:
		{
			struct connack_packet_header *header = (struct connack_packet_header *)buf + 1 + l;
			if (!header->ca_flag.reserved) {
				result->content.connack.sp = header->ca_flag.sp;
				result->content.connack.cr_code = header->cr_code;
			} else {
				result->error = CONTROL_PACKET_ERROR_INVALID_RESERVE;
			}
		}
			break;
		case CONTROL_PACKET_TYPE_PUBLISH:
			parse_publish_packet(buf + 1 + l, remain, fixed->flag, result);
			break;
		case CONTROL_PACKET_TYPE_PUBACK:
			result->content.packet_id = ((uint16_t)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PUBREC:
			result->content.packet_id = ((uint16_t)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PUBREL:
			result->content.packet_id = ((uint16_t)*(buf + 1 + l) << 8) | *(buf + 2 + l);
			break;
		case CONTROL_PACKET_TYPE_PUBCOMP:
			result->content.packet_id = ((uint16_t)*(buf + 1 + l) << 8) | *(buf + 2 + l);
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
			result->content.packet_id = ((uint16_t)*(buf + 1 + l) << 8) | *(buf + 2 + l);
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

void parse_connect_packet(uint8_t *buf, uint32_t len, struct parse_result *result)
{
	struct connect_packet_header *header = (struct connect_packet_header *)buf;
	if (check_connect_packet_name(header->pname)) {
		result->error = CONTROL_PACKET_ERROR_WRONG_NAME;
		return;
	}
	result->content.connect.level = header->level;
	result->content.connect.flags = header->flags;
	result->content.connect.alive = header->alive;
	buf += sizeof(*header);
	result->content.connect.client_id.len = ((uint16_t)(buf[0]) << 8) | buf[1];
	memcpy(result->content.connect.client_id.data, buf + 2, ((uint16_t)(buf[0]) << 8) | buf[1]);
	buf += 2 + (((uint16_t)(buf[0]) << 8) | buf[1]);
	if (header->flags.wf) {
		result->content.connect.w_topic.len = ((uint16_t)(buf[0]) << 8) | buf[1];
		memcpy(result->content.connect.w_topic.data, buf + 2, ((uint16_t)(buf[0]) << 8) | buf[1]);
		buf += 2 + (((uint16_t)(buf[0]) << 8) | buf[1]);
		result->content.connect.w_mesg.len = ((uint16_t)(buf[0]) << 8) | buf[1];
		memcpy(result->content.connect.w_mesg.data, buf + 2, ((uint16_t)(buf[0]) << 8) | buf[1]);
		buf += 2 + (((uint16_t)(buf[0]) << 8) | buf[1]);
	}
	if (header->flags.unf) {
		result->content.connect.u_name.len = ((uint16_t)(buf[0]) << 8) | buf[1];
		memcpy(result->content.connect.u_name.data, buf + 2, ((uint16_t)(buf[0]) << 8) | buf[1]);
		buf += 2 + (((uint16_t)(buf[0]) << 8) | buf[1]);
	}
	if (header->flags.pf) {
		result->content.connect.pwd.len = ((uint16_t)(buf[0]) << 8) | buf[1];
		memcpy(result->content.connect.pwd.data, buf + 2, ((uint16_t)(buf[0]) << 8) | buf[1]);
		buf += 2 + (((uint16_t)(buf[0]) << 8) | buf[1]);
	}
}

void parse_publish_packet(uint8_t *buf, uint32_t len,  uint8_t flag, struct parse_result *result)
{
	/* TO BE IMPLEMENT. */
}
