/*
 * system.h
 *
 *  Created on: 2017年6月19日
 *      Author: work
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <unistd.h>
#include <stdio.h>

typedef struct
{
	__u8 firmware_ver[3];
	__u8 hardware_ver[3];
	__u8 protocol_ver[3];
	__u8 mac[6];
	__u8 time_valid;
	__s16 timezone;
//	__u32 utc_time;
	__u8 poweron;
}system_info_t;

void sys_context_init();
int getSystemTime();

#endif /* SYSTEM_H_ */
