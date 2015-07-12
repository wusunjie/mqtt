/*
 * mqtt_types.h
 *
 *  Created on: 2015Äê7ÔÂ11ÈÕ
 *      Author: Administrator
 */

#ifndef MQTT_TYPES_H_
#define MQTT_TYPES_H_

typedef unsigned char mqtt_type_uint8;
typedef unsigned short int mqtt_type_unicode;
typedef unsigned short int mqtt_type_uint16;
typedef long mqtt_type_uint32;

struct mqtt_type_utf8 {
	mqtt_type_uint16 len;
	mqtt_type_uint8 *data;
};

#endif /* MQTT_TYPES_H_ */
