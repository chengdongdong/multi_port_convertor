/*
 * net.h
 *
 *  Created on: 2017年3月21日
 *      Author: work
 */

#ifndef NET_H_
#define NET_H_

#include "list.h"

#define LINK_ON		1
#define LINK_OFF		0
#define YES					1
#define NO					0


typedef struct
{
	struct list_head list;
	struct Buffer *buf;
	__u8   data[128];
}net_send_format_t;


typedef struct
{
	__u8 state;
	__u8 course;
	__u8 err;
}net_state_t;


void net_context_init();

#endif /* NET_H_ */
