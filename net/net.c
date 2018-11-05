/*
 * net.c
 *
 *  Created on: 2017年3月21日
 *      Author: work
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

#include "thread.h"
#include "mailbox.h"
#include "tcp.h"
#include "buf_factory.h"
#include "net.h"
#include "debug.h"
#include "net_protocol.h"

#define handle_receive_error(x) \
			if (x <= 0) \
			{ \
      client_break_socket();\
				goto RECONNECT; \
			}


extern net_state_t  net_state[4];

/* 网络连接状态 */
__u32 g_conn_state = LINK_OFF;
pthread_mutex_t g_mutex_conn_state;
__s32 g_net_mbx_id = 0;
sem_t g_sem_conn_state;
struct buffer_factory *g_net_send_buf = NULL;

pthread_mutex_t http_poll_mutex;
struct list_head g_http_send_list;

#define PORT 6666
void net_connect_tsk()
{
	__s32 ret = 0;
	char *pc_ip_addr = "192.168.3.101";

	while(1)
	{
		/* 创建客户端socket, 并向服务器发起连接 */
		tcp_client_create_socket();
		printf("connect with server...");
		fflush(stdout);
		while (1)
		{
			ret = client_connect_server((__s8 *)pc_ip_addr);
			if (ret != 0)
			{
				usleep(500000);
				printf(".");
				fflush(stdout);
			}
			else
			{
				pthread_mutex_lock(&g_mutex_conn_state);
				g_conn_state = LINK_ON;
				pthread_mutex_unlock(&g_mutex_conn_state);

				sem_post(&g_sem_conn_state);
				printf("\n tcp link succeed.\n");
				break;
			}
		}

		//测试代码
//		net_send_info_t  info;
//		info.cmd_id = 0xAF;
//		info.len = 10;
//		info.data[0] = 1;
//		info.data[1] = 2;
//		info.data[2] = 3;
//		info.data[3] = 4;
//		info.data[4] = 5;
//		info.data[5] = 6;
//		info.data[6] = 7;
//		info.data[7] = 8;
//		info.data[8] = 9;
//		info.data[9] = 10;
//
//		while(1)
//		{
//			net_protocol_dispose(&info);
//			usleep(10000);
//		}
//-----------------------------------------------------------------------------------------------------

		while (1)
		{
			if((check_link_status() == 0) && (g_conn_state == LINK_ON))
			{
				sleep(1);
			}
			else
			{
				break;
			}
		}
	}
}

void net_send_tsk()
{
	net_send_format_t *Data_OutMbx;
	__u32 data_len = 0;
	__u32 nhas_send = 0;
	__s32 nsend;
	__u32 link_state = 0;

	struct message msg;

	msg.id = g_net_mbx_id;
	msg.prio = 0;

	while (1)
	{
		mailbox_pend(&msg);

		data_len = msg.data_len;
		Data_OutMbx = msg.p;

		pthread_mutex_lock(&g_mutex_conn_state);
		link_state = g_conn_state;
		pthread_mutex_unlock(&g_mutex_conn_state);
		if (link_state == LINK_OFF)
		{
			buf_factory_recycle(0, Data_OutMbx->buf, g_net_send_buf);
			continue;
		}

		nhas_send = 0;
		while (nhas_send < data_len)
		{
			nsend = client_send_data(Data_OutMbx->data + nhas_send, data_len - nhas_send);
			if (nsend <= 0)
			{
				MSG("goto OUT\n");
				break;
			}
			nhas_send = nhas_send + nsend;
		}

		buf_factory_recycle(0, Data_OutMbx->buf, g_net_send_buf);
	}
}


__s8 net_info[128];
void net_recv_tsk()
{
	__u32 nhas_received = 0;
	__s32 nrecv = 0;

RECONNECT:
	sem_wait(&g_sem_conn_state);

	while(1)
	{
		/* 接收长度和帧头 */
		nhas_received = 0;
		while (nhas_received < 128)
		{
			nrecv = client_recv_data(&net_info, 128 - nhas_received);
			handle_receive_error(nrecv);
			nhas_received += nrecv;
		}
	}
}


extern __s32  udp_socket_create();
extern __s32  check_dhcp_state(const char* ifname);
void net_detect_tsk()
{
	char  buf[20];
	FILE   *stream;
	__s32 ret;
	udp_socket_create();

	while(1)
	{
		sleep(1);

		//网线检测
		memset( buf, 0, sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
		stream = popen( "devmem 0x10110080", "r" );
		fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
		pclose( stream );
//		printf("buf = %s\n", buf );


		if(*(buf+3) == '3')
		{
			//网络在线
			net_state[0].course = 1;//网线连接上
//			printf("net link is running :\n");
			while(1)
			{
				ret = check_dhcp_state("br-lan");
				if (ret != 0)
					sleep(1);
				else
				{
					net_state[0].course = 2;//DHCP成功
					break;
				}
			}
		}

		else if(*(buf+3) == '1')
		{
			//网络不在线
			net_state[0].course = 0;
			net_state[0].err = 1;//网线断开
//			printf("net link is no running :\n");
		}

		//wifi状态检测
		memset( buf, 0, sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
		stream = popen( "ap_client", "r" );
		fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
		pclose( stream );
//		printf("buf = %s\n", buf );

		if(strcmp(buf, "ok") >= 0)
		{
			//网络在线
			net_state[1].course = 1;//网连接上
			printf("wifi is running :\n");
			while(1)
			{
				ret = check_dhcp_state("apcli0");
				if (ret != 0)
					sleep(1);
				else
				{
					net_state[1].course = 2;//DHCP成功
					break;
				}
			}
		}

		else if(strcmp(buf, "no") >= 0)
		{
			//网络不在线
			net_state[1].course = 0;
			net_state[1].err = 1;//wifi断开
			printf("wifi is no running :\n");
		}
	}
}


void net_context_init()
{
//	pthread_mutex_init(&g_mutex_conn_state, NULL);
//
//	sem_init(&g_sem_conn_state, 0, 0);
//
//	g_net_mbx_id = mailbox_create("/net", 16);
//
//	if(create_buf_factory(16, sizeof(net_send_format_t), &g_net_send_buf) != 0)
//	{
//		ERROR("net_buf creat error");
//		return ;
//	}
//
//	//锁初始化
//	pthread_mutex_init(&http_poll_mutex,NULL);
//
//	//链表初始化
//	init_list_head(&g_http_send_list);

//	add_new_thread(NULL, (void *)&net_connect_tsk, 16, 8*1024);
//	add_new_thread(NULL, (void *)&net_send_tsk, 17, 8*1024);
//	add_new_thread(NULL, (void *)&net_recv_tsk, 15, 8*1024);

	add_new_thread(NULL, (void *)&net_detect_tsk, 15, 8*1024);
}
