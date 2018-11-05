/*
 * uart_communicat.h
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */

#ifndef UART_COMMUNICAT_H_
#define UART_COMMUNICAT_H_

#include "list.h"
#include <linux/types.h>


#define UART_PROCOTOL_MAX_LEN    128+1


typedef struct
{
	__u8 head;
	__u8 length_msb;
	__u8 length_lsb;
	__u8 sta_param;
	__u8 sn;
	__u8 pdt_type;
	__u8 cmd_id;
}m_cmd_header_t;


typedef struct
{
	__u32   delay;
	__u32   times;
	__u32   type;    //0:消息应答      //1:主动发送
	__u32   free_flag;
}uart_poll_t;


typedef struct
{
	m_cmd_header_t  header;
	__u8 data[UART_PROCOTOL_MAX_LEN];
} uart_cmd_format_t;


typedef struct
{
	struct list_head list;
	struct Buffer *buf;
	uart_poll_t poll;
	uart_cmd_format_t cmd;
}uart_send_format_t;


void uart_context_init(void);

#endif /* UART_COMMUNICAT_H_ */


