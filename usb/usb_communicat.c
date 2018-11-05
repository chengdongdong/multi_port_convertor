/*
 * usb_communicat.c
 *
 *  Created on: 2017年2月27日
 *      Author: work
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <linux/types.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "debug.h"
#include "buf_factory.h"
#include "mailbox.h"
#include "usb.h"
#include "hid_usage_id.h"
#include "uart_protocol.h"
#include "usb_communicat.h"


#define HID_TIME_OUT 10
#define NSECTOSEC    1000000000

__s32 g_hid_mbx_id= 0;
struct buffer_factory *g_hid_buf = NULL;
sem_t sem_hid_data;
int hid_fd = 0;
static pthread_mutex_t mutex_hid_timer;
static __u32 timer = 0;

__u8 hid_key_buf[128];
__u32 hid_key_length = 0;
static __u8 hid_dev_type = 0;

static __s32 hid_read(void* buffer, int buffer_size)
{
	if(buffer == NULL)
    {
		perror("hid_read::pointer error!");
		return -1;
    }
	return read(hid_fd, buffer, buffer_size);
}

__u8 find_hid_type(__s16 product)
{

	const usb_product_list_t *list;
	for(list=usb_product_list; list->product!=0; list++)
	{
		if(product == list->product)
			return list->usb_type;
	}
	return 0;
}


void hid_reopen_read_tsk(void)
{
	struct Buffer *p_buf;
	struct message msg;
	hid_format_t* hidpdata;
	int read_len = 0;
	__s16 product;

//	printf("file:%s line:%d\n",__FILE__, __LINE__);
	while(1)
	{
		while(1)
		{
			hid_fd = open_hid();
			if(hid_fd <= 0)
			{
//				printf("file:%s line:%d\n",__FILE__, __LINE__);
				usleep(1000);
				continue;
			}
//			printf("file:%s line:%d\n",__FILE__, __LINE__);
			printf("hid_fd = %d\n",hid_fd);

			set_hid(hid_fd, &product);

			hid_dev_type = find_hid_type(product);
			if(hid_dev_type == 0)
			{
				perror("hid_type error!");
				close(hid_fd);
			}
			else
			{
//				printf("file:%s line:%d\n",__FILE__, __LINE__);
				hid_insert_ntf(hid_dev_type);
//				printf("file:%s line:%d\n",__FILE__, __LINE__);
				break;
			}
		}

		while(1)
		{
			p_buf = (void*)buf_factory_produce(g_hid_buf);
			if(p_buf == NULL)
			{
				ERROR("buf_factory_produce");
			}
			hidpdata = (hid_format_t*)(p_buf->memory);
			hidpdata->buf = p_buf;

			read_len = hid_read(&(hidpdata->key_word), 8);
			if(read_len <= 0)
			{
				buf_factory_recycle(0, p_buf, g_hid_buf);
				close(hid_fd);
				hid_pullout_ntf(hid_dev_type);
				break;
			}

			msg.id = g_hid_mbx_id;
			msg.prio = 0;
			msg.p = hidpdata;
			msg.data_len = sizeof(hid_format_t);

			mailbox_post(&msg);
		}
	}
}


//void timer_handler()
//{
////	printf("file:%s line:%d\n",__FILE__, __LINE__);
//	pthread_mutex_lock(&mutex_hid_timer);
//	timer++;
//	pthread_mutex_unlock(&mutex_hid_timer);
//}

void setTimer(int seconds, int useconds)
{
	struct timeval temp;

	temp.tv_sec = seconds;
	temp.tv_usec = useconds;
	select(0, NULL, NULL, NULL, &temp);
}

void hid_timer_tsk()
{
	while(1)
	{
		setTimer(0, 1000);

		pthread_mutex_lock(&mutex_hid_timer);
		timer++;
		pthread_mutex_unlock(&mutex_hid_timer);
	}
}


void hid_data_tsk()
{
	__s32 ret;
	void* pcmp;
	__u32 i;
	hid_format_t *Data_OutMbx;
	struct message msg;
	struct timespec tm;

	msg.id = g_hid_mbx_id;
	msg.prio = 0;

	memset(hid_key_buf, 0, 128);

	while(1)
	{
		clock_gettime(CLOCK_REALTIME, &tm);
		tm.tv_sec += 0;
		tm.tv_nsec += 1000000;
		tm.tv_sec += tm.tv_nsec/NSECTOSEC;
		tm.tv_nsec = tm.tv_nsec%NSECTOSEC;

		ret = mailbox_timedpend(&msg, &tm);
		if(ret > 0)
		{
			pthread_mutex_lock(&mutex_hid_timer);
			timer = 0;
			pthread_mutex_unlock(&mutex_hid_timer);

			Data_OutMbx = msg.p;

			if(Data_OutMbx->key_word.head == 1) //刷卡器
			{
				pcmp = &(Data_OutMbx->key_word.code);
			}
			else //扫码枪
			{
				pcmp = &(Data_OutMbx->key_word.head);
			}

			for(i = 0; hid_id_tab[i].ascii != 0; i++)
			{
				if(memcmp(hid_id_tab[i].id, pcmp, 3) == 0)
				{
					if(hid_id_tab[i].ascii != 13)
					{
//						printf("card key = %c  \n",hid_id_tab[i].ascii);
						hid_key_buf[hid_key_length] = hid_id_tab[i].ascii;
						hid_key_length++;
						break;
					}
					else
					{
						hid_key_buf[hid_key_length] = 0;
						printf("0D card key = %s  \n",hid_key_buf);
						hid_data_ntf(hid_key_buf, hid_key_length, hid_dev_type);
						hid_key_length = 0;
						break;
					}
				}
				else
					continue;
			}
			buf_factory_recycle(0, Data_OutMbx->buf, g_hid_buf);
		}

		else
		{
			if(timer  >= HID_TIME_OUT)
			{
				if(hid_key_length)
				{
					hid_key_buf[hid_key_length] = 0;
					printf("card key = %s  \n",hid_key_buf);
					hid_data_ntf(hid_key_buf, hid_key_length, hid_dev_type);
					hid_key_length = 0;
				}
				pthread_mutex_lock(&mutex_hid_timer);
				timer = 0;
				pthread_mutex_unlock(&mutex_hid_timer);
			}
//			printf("file:%s line:%d\n",__FILE__, __LINE__);
		}
	}
}




void hid_context_init(void)
{
//	struct itimerval tv, otv;
//
//	signal(SIGALRM, timer_handler);
//	tv.it_value.tv_sec = 0;
//	tv.it_value.tv_usec = 1;
//	tv.it_interval.tv_sec = 0;
//	tv.it_interval.tv_usec = 1000;
//	setitimer(ITIMER_REAL, &tv, &otv);
//	__s16 product;
//
//	hid_fd = open_hid();
//	printf("hid_fd = %d\n",hid_fd);
//
//	set_hid(hid_fd, &product);
//
//	hid_dev_type = find_hid_type(product);
//	if(hid_dev_type == 0)
//		perror("hid_type error!");

	//信号量初始化
	sem_init(&sem_hid_data, 0, 0);

	//初始化锁
	pthread_mutex_init(&mutex_hid_timer,NULL);

	//缓冲初始化
	if(create_buf_factory(16, sizeof(hid_format_t), &g_hid_buf) != 0)
	{
		ERROR("uart_buf creat error");
		return ;
	}

	//消息队列初始化
	g_hid_mbx_id  = mailbox_create("/mq_hid", 16);

	//线程初始化
	add_new_thread(NULL, (void *) &hid_reopen_read_tsk, 21, 8*1024);
	add_new_thread(NULL, (void *) &hid_timer_tsk, 20, 8*1024);
	add_new_thread(NULL, (void *) &hid_data_tsk, 22, 8*1024);
}
