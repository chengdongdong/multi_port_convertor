/*
 * uart_communicat.c
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */
#include <linux/types.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "uart.h"
#include "uart_communicat.h"
#include "uart_protocol.h"
#include "thread.h"
#include "debug.h"
#include "buf_factory.h"
#include "mailbox.h"

#define getresplen()    \
	((parray->cmd.header.length_msb<<8) + parray->cmd.header.length_lsb + 4)



int uart_fd = 0;
struct buffer_factory *g_uart_send_buf = NULL;

pthread_mutex_t uart_poll_mutex;
struct list_head g_uart_send_list;
__s32 g_uart_mbx_id= 0;
uart_cmd_format_t uart_recv_buf;



void uart_send_tsk()
{
	uart_send_format_t *Data_OutMbx;
	struct message msg;
	__u32 data_len = 0;
	__u32 nhas_send = 0;
	__s32 nsend;

	msg.id = g_uart_mbx_id;
	msg.prio = 0;

	while(1)
	{
		mailbox_pend(&msg);
		Data_OutMbx = msg.p;
		data_len = msg.data_len;
		nhas_send = 0;

		while (nhas_send < data_len)
		{
			nsend = write(uart_fd, &(Data_OutMbx->cmd) + nhas_send, data_len - nhas_send);
			if (nsend <= 0)
			{
				break;
			}
			nhas_send = nhas_send + nsend;
		}

		if(Data_OutMbx->poll.free_flag)
		{
			memset(&(Data_OutMbx->poll), 0, sizeof(uart_poll_t));
			buf_factory_recycle(0, Data_OutMbx->buf, g_uart_send_buf);
		}
	}
}

__u32 app_uart_get(__u8 * p_byte)
{
	__s32 nrecv;
	nrecv = read(uart_fd, p_byte, 1);
	if(nrecv == 1)
		return 0;
	else
		return 1;
}


__u32 uart_get_data(void)
{
	__u8 ch;
	static __u32 proctol_start = false;
	static __u32 recv_len = 0;
	static __u8* recv_data_p = (__u8*)(&uart_recv_buf);
	static __u32 data_len = 0;

	while(!app_uart_get(&ch))
	{
//		printf("file:%s line:%d\n",__FILE__, __LINE__);
		if((ch == 0xC0)&&(proctol_start == false))
		{
			proctol_start = true;
		}

		if(proctol_start == true)
		{
			recv_data_p[recv_len++] = ch;

			if(recv_len == 3)
			{
				data_len = (recv_data_p[1]<<8) + recv_data_p[2] + 1;
				if(data_len == 0)
				{
					recv_len = 0;
					data_len = 0;
					proctol_start = false;
					return true;
				}
				else if(data_len > UART_PROCOTOL_MAX_LEN)
				{
					recv_len = 0;
					data_len = 0;
					proctol_start = false;
					return false;
				}
			}

			else if(recv_len > 3)
			{
				data_len--;

				if(data_len == 0)
				{
					recv_len = 0;
					proctol_start = false;
					return true;
				}
			}
		}
		else
		{
			continue;
		}
	}
	return false;
}




void uart_recv_tsk()
{
	while(1)
	{
		if(!(uart_get_data()))
		{
//			usleep(1000);
			continue;
		}
		uart_protocol_parse(&uart_recv_buf);
	}
}

void uart_poll_tsk()
{
	struct list_head *p;
	uart_send_format_t *parray;
	struct message msg;

	while(1)
	{
		usleep(5000);
//		sleep(1);

		pthread_mutex_lock(&uart_poll_mutex);
		p = g_uart_send_list.next;
		while(p != &g_uart_send_list)
		{
			parray = (uart_send_format_t *)(p->owner);
			if(parray->poll.type == 0)//如果是数据回应
			{
				parray->poll.free_flag = 1;

				msg.id = g_uart_mbx_id;
				msg.prio = 0;
//				msg.data = &(parray->cmd);
				msg.p = parray;
				msg.data_len = getresplen();
				mailbox_post(&msg);

				p = p->next;
				list_del(p->prev);
				continue;
			}
			else if(parray->poll.type == 1)//如果是主动发送
			{

				if(parray->poll.times == 0)//首次发送
				{
//					printf("file:%s line:%d\n",__FILE__, __LINE__);
					msg.id = g_uart_mbx_id;
					msg.prio = 0;
//					msg.data = &(parray->cmd);
					msg.p = parray;
					msg.data_len = getresplen();
					mailbox_post(&msg);

					parray->poll.times++;
				}

				else
				{
					if(parray->poll.free_flag)//如果已经收到回应
					{
						p = p->next;
						list_del(p->prev);
						memset(&(parray->poll), 0, sizeof(uart_poll_t));
						buf_factory_recycle(0, parray->buf, g_uart_send_buf);
						continue;
					}

					parray->poll.delay += 5;
					if(parray->poll.delay >= 500)
					{
//						printf("file:%s line:%d\n",__FILE__, __LINE__);
						parray->poll.delay = 0;
						parray->poll.times++;
						if(parray->poll.times>3)
						{
							p = p->next;
							list_del(p->prev);
							memset(&(parray->poll), 0, sizeof(uart_poll_t));
							buf_factory_recycle(0, parray->buf, g_uart_send_buf);
							continue;
						}

						msg.id = g_uart_mbx_id;
						msg.prio = 0;
//						msg.data = &(parray->cmd);
						msg.p = parray;
						msg.data_len = getresplen();
						mailbox_post(&msg);

					}
				}
			}
			p = p->next;
		}
		pthread_mutex_unlock(&uart_poll_mutex);
	}
}


void uart_context_init(void)
{
	//打开串口
	if((uart_fd = open_port()) < 0)
	{
		ERROR("open_port error");
		return;
	}

	//设置波特率
	if((set_opt(uart_fd,115200,8,'N',1)) < 0)
	{
		ERROR("set_opt error");
		return;
	}

	//链表初始化
	init_list_head(&g_uart_send_list);

	//缓冲初始化
	if(create_buf_factory(16, sizeof(uart_send_format_t), &g_uart_send_buf) != 0)
	{
		ERROR("uart_buf creat error");
		return ;
	}

	//消息队列初始化
	g_uart_mbx_id  = mailbox_create("/mq_uart", 16);

	//锁初始化
	pthread_mutex_init(&uart_poll_mutex,NULL);

//	printf("file:%s line:%d\n",__FILE__, __LINE__);

	//线程初始化
	add_new_thread(NULL, (void *) &uart_send_tsk, 19, 8*1024);
	add_new_thread(NULL, (void *) &uart_recv_tsk, 17, 8*1024);
	add_new_thread(NULL, (void *) &uart_poll_tsk, 18, 8*1024);

}
