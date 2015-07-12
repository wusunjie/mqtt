/*
 * main.c
 *
 *  Created on: 2015Äê7ÔÂ10ÈÕ
 *      Author: Administrator
 */

#include "control_packet.h"

#pragma pack(push, 1)
struct data {
	unsigned char msb;
	unsigned char lsb;
	union {
		unsigned char bytes;
		struct {
			unsigned char lsb:4;
			unsigned char msb:4;
		} bits;
	} field;
};
#pragma pack(pop)

int main(void)
{
	struct data x;
	unsigned char *cur = (unsigned char *)&x;
	cur[0] = 0x12;
	cur[1] = 0x34;
	cur[2] = 0x56;
	printf("%x, %x, %x\n", x.msb, x.lsb, (unsigned short)cur[0] << 8 | cur[1]);
	printf("%x, %x\n", x.field.bits.msb, x.field.bits.lsb);
	return 0;
}
