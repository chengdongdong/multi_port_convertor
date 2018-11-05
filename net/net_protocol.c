/*
 * net_protocol.c
 *
 *  Created on: 2017年3月21日
 *      Author: work
 */

#include <stddef.h>
#include <string.h>

#include "net_protocol.h"
#include "buf_factory.h"
#include "mailbox.h"
#include "debug.h"
#include "net.h"

extern struct buffer_factory *g_net_send_buf;
extern __s32 g_net_mbx_id;

__s32 net_protocol_dispose(net_send_info_t* info)
{
//	__u32 protocol_len;
	net_send_format_t*  sendpdata;
	struct Buffer *p_buf;
	struct message msg;

	p_buf = (void*)buf_factory_produce(g_net_send_buf);
	if(p_buf == NULL)
	{
		ERROR("buf_factory_produce");
		return -1;
	}

	sendpdata = (net_send_format_t*)(p_buf->memory);
	sendpdata->buf = p_buf;

	if(info->len)
	{
//		printf("protocol_len: %d, info->len: %d\n", protocol_len, info->len);
		memcpy(sendpdata->data, info->data, info->len);
	}

	msg.id = g_net_mbx_id;
	msg.prio = 0;
	msg.p = sendpdata;
	msg.data_len = info->len;

	mailbox_post(&msg);

	return 0;
}
