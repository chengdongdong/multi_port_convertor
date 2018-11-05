/*
 * net_protocol.h
 *
 *  Created on: 2017年3月21日
 *      Author: work
 */

#ifndef NET_PROTOCOL_H_
#define NET_PROTOCOL_H_

#include <linux/types.h>

typedef struct
{
	__u8 	 cmd_id;       //命令字，具体定义见扩展协议2.3
	__u32  len;          //所传数据长度，如果没有数据则赋值0
	__u8   data[128];    //所传数据
}net_send_info_t;

__s32 net_protocol_dispose(net_send_info_t* info);

#endif /* NET_PROTOCOL_H_ */
