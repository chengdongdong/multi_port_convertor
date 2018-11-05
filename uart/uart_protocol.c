/*
 * uart_protocol.c
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */

#include <stddef.h>
#include <linux/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "net_protocol.h"
#include "uart_protocol.h"
#include "uart_communicat.h"
#include "buf_factory.h"
#include "mailbox.h"
#include "debug.h"
#include "net.h"
#include "system.h"
#include "cJSON.h"
#include "http_protocol.h"

#define setresplen( len )														\
{			                                                                    \
	Respdata->header.length_msb = len>>8;                                       \
	Respdata->header.length_lsb = len;                                          \
}

extern system_info_t  system_info;
extern net_state_t  net_state[4];

extern __s32 g_uart_mbx_id;
extern struct buffer_factory *g_uart_send_buf;
extern struct list_head g_uart_send_list;
extern pthread_mutex_t uart_poll_mutex;

uart_cmd_format_t *Data_p;
static __u16  Respdatalen = 0;
static uart_send_format_t*  sendpdata = NULL;
static uart_cmd_format_t*  Respdata = NULL;
static __u8 uart_send_sn = 0;
static __u8 http_send_sn = 0;

__u8 check_sum_cal(void *p, __u16 len);


static __s32 uart_protocol_dispose(uart_send_info_t* info)
{
	__u32 protocol_len;
	static uart_cmd_format_t*  Respdata;
	uart_send_format_t*  sendpdata;
	struct Buffer *p_buf;
//	struct message msg;

	p_buf = (void*)buf_factory_produce(g_uart_send_buf);
	if(p_buf == NULL)
	{
		ERROR("buf_factory_produce");
		return -1;
	}

	sendpdata = (uart_send_format_t*)(p_buf->memory);
	sendpdata->buf = p_buf;
	Respdata = &(sendpdata->cmd);
	Respdata->header.head = 0xD0;
	protocol_len = info->len + 4;
	setresplen(protocol_len);
	Respdata->header.sta_param = 0;
	Respdata->header.sn = uart_send_sn;
	uart_send_sn++;
	Respdata->header.pdt_type = info->pdt_type;
	Respdata->header.cmd_id = info->cmd_id;

	if(info->len)
	{
//		printf("protocol_len: %d, info->len: %d\n", protocol_len, info->len);
		memcpy(Respdata->data, info->data, info->len);
	}
	Respdata->data[info->len] = check_sum_cal(&(Respdata->header.sta_param), protocol_len);

	sendpdata->poll.type = info->poll_type;
	sendpdata->list.owner = sendpdata;

	pthread_mutex_lock(&uart_poll_mutex);
	list_add_tail(&(sendpdata->list), &g_uart_send_list);
	pthread_mutex_unlock(&uart_poll_mutex);

	return 0;
}

//主动上传类命令
void poweron_ntf(__u8 state)
{
	uart_send_info_t  info;
	info.len = 0;
	info.data[info.len] = state;
	info.len += 1;
	memcpy(&(info.data[info.len]), system_info.firmware_ver, 3);
	info.len += 3;
	memcpy(&(info.data[info.len]), system_info.hardware_ver, 3);
	info.len += 3;
	memcpy(&(info.data[info.len]), system_info.protocol_ver, 3);
	info.len += 3;
	memcpy(&(info.data[info.len]), system_info.mac, 6);
	info.len += 6;
	info.cmd_id = NTF_POWERON;
	info.pdt_type = 0xFF;
	info.poll_type = 1;
	uart_protocol_dispose(&info);
}

void err_sta_ntf(__u8 state, __u8 course, __u8 err)
{
	uart_send_info_t  info;
	info.len = 3;
	info.data[0] = state;
	info.data[1] = course;
	info.data[2] = err;
	info.cmd_id = NTF_ERR_STA;
	info.pdt_type = 0xFF;
	info.poll_type = 0;
	uart_protocol_dispose(&info);
}

void hid_insert_ntf(__u8 dev_type)
{
	uart_send_info_t  info;
	info.len = 1;
	info.data[0] = dev_type;
	info.cmd_id = NTF_HID_INSERT;
	info.pdt_type = 0xA1;
	info.poll_type = 1;
	uart_protocol_dispose(&info);
}

void hid_pullout_ntf(__u8 dev_type)
{
	uart_send_info_t  info;
	info.len = 1;
	info.data[0] = dev_type;
	info.cmd_id = NTF_HID_PULLOUT;
	info.pdt_type = 0xA1;
	info.poll_type = 1;
	uart_protocol_dispose(&info);
}

void hid_data_ntf(__u8 *data, __u32 length, __u8 dev_type)
{
	uart_send_info_t  info;
	info.len = length+1;
	info.data[0] = dev_type;
	memcpy(&(info.data[1]) ,data, length);
	info.cmd_id = NTF_HID_DATA;
	info.pdt_type = 0xA1;
	info.poll_type = 1;
	uart_protocol_dispose(&info);
}

//void print_sta_ntf(__u8 state)
//{
//	uart_send_info_t  info;
//	info.len = 1;
//	info.data[0] = state;
//	info.cmd_id = NTF_PRINT_STA;
//	info.pdt_type = 0xA1;
//info.poll_type = 0;
//	uart_protocol_dispose(&info);
//}

void http_up_data_ntf(__u8 state)
{
	uart_send_info_t  info;
	info.len = 1;
	info.data[0] = state;
	info.cmd_id = NTF_HTTP_UP_DATA;
	info.pdt_type = 0xA1;
	info.poll_type = 0;
	uart_protocol_dispose(&info);
}

//接收类命令

static void poweron_rsp(void)
{
	;
}
__u8 const poweron_rsp_argvlist[]={1,0};


static void que_sta_rsp(void)
{
	__u32 ptl_len = 0;
	__u32 i;
	ptl_len = (Data_p->header.length_msb<<8) + Data_p->header.length_lsb;
	for(i = 0; i < (ptl_len-4); i++)
	{
		if(Data_p->data[i] < 4)
		{
			memcpy(&(Respdata->data[Respdatalen]), &(net_state[Data_p->data[i]]), sizeof(net_state_t));
			Respdatalen += sizeof(net_state_t);
		}
	}
}
__u8 const que_sta_rsp_argvlist[]={1,0};

static void time_sync_rsp(void)
{
	struct timeval tv;
	struct tm *tp;

	gettimeofday (&tv , NULL);
	tp=localtime(&(tv.tv_sec));

	printf("timezone = %d\n",  system_info.timezone);
	printf("local time = %d-%d-%d %d:%d:%d\n", tp->tm_year+1900,tp->tm_mon,tp->tm_mday,tp->tm_hour,tp->tm_min ,tp->tm_sec);

	Respdata->data[Respdatalen] = system_info.time_valid;
	Respdatalen += 1;

	Respdata->data[Respdatalen] = system_info.timezone>>8;
	Respdatalen += 1;
	Respdata->data[Respdatalen] = system_info.timezone;
	Respdatalen += 1;
	Respdata->data[Respdatalen] = tv.tv_sec>>24;
	Respdatalen += 1;
	Respdata->data[Respdatalen] = tv.tv_sec>>16;
	Respdatalen += 1;
	Respdata->data[Respdatalen] = tv.tv_sec>>8;
	Respdatalen += 1;
	Respdata->data[Respdatalen] = tv.tv_sec;
	Respdatalen += 1;
}
__u8 const time_sync_rsp_argvlist[]={1,0};



void drv_poweron_rsp()
{
	Respdata->data[Respdatalen] = system_info.poweron;
	Respdatalen += 1;
	memcpy(&(Respdata->data[Respdatalen]), system_info.firmware_ver, 3);
	Respdatalen += 3;
	memcpy(&(Respdata->data[Respdatalen]), system_info.hardware_ver, 3);
	Respdatalen += 3;
	memcpy(&(Respdata->data[Respdatalen]), system_info.protocol_ver, 3);
	Respdatalen += 3;
	memcpy(&(Respdata->data[Respdatalen]), system_info.mac, 6);
	Respdatalen += 6;
}
__u8 const drv_poweron_rsp_argvlist[]={1,0};

static void hid_insert_rsp(void)
{
	;
}
__u8 const hid_insert_rsp_argvlist[]={1,0};


static void hid_pullout_rsp(void)
{
	;
}
__u8 const hid_pullout_rsp_argvlist[]={1,0};


static void hid_data_rsp(void)
{
	;
}
__u8 const hid_data_rsp_argvlist[]={1,0};


static void print_sta_rsp(void)
{
	;
}
__u8 const print_sta_rsp_argvlist[]={1,0};


static void print_ask(void)
{
	__u8 print_buf[1024];
	__u32 n;
	FILE   *stream;

	__s32 ret = 0;
	char absolute_file_name[32] = "/dev/usb/lp0";
	ret = access(absolute_file_name, F_OK);

	if(ret)
	{
		Respdata->data[Respdatalen] = 0;
		Respdatalen += 1;
	}
	else
	{
		Respdata->data[Respdatalen] = 2;
		Respdatalen += 1;
		return;
	}

  n = 0;
  n+=sprintf((char*)print_buf+n,"echo \"");
  n+=sprintf((char*)print_buf+n,(char*)(Data_p->data));
  n+=sprintf((char*)print_buf+n,"\" > /dev/usb/lp0");
  stream = popen((__const char *)print_buf, "r");
  pclose(stream);
}
__u8 const print_ask_argvlist[]={1,0};


static void http_up_data_rsp(void)
{
		__u16 tmp;
		__u32 p;
		struct tm* tm_p;
		__u8 buf[100];
		cJSON *root,*fmt, *list;
		__u32 utc_time;
		char  szBuffer[100];
		__s16 timezone;

		root = cJSON_CreateObject();

		//发送序列号
		sprintf(szBuffer, "%d", http_send_sn);
//		cJSON_AddItemToObject(root, "queue_num", cJSON_CreateNumber(http_send_sn));
		cJSON_AddItemToObject(root, "queue_num", cJSON_CreateString(szBuffer));
		http_send_sn++;

		//data
//		cJSON_AddItemToObject(root, "data", fmt=cJSON_CreateObject());
		cJSON_AddItemToObject(root, "data", fmt=cJSON_CreateArray());
		cJSON_AddItemToArray(fmt, list = cJSON_CreateObject());
		//card_no
		cJSON_AddStringToObject(list,"card_no",  (char*)(Data_p->data));

		//时区
		p = strlen((char*)(Data_p->data));
		p += 1;
		timezone = ((Data_p->data[p]<<8)|Data_p->data[p+1])/2;
		if(timezone>=0)
		{
			if(timezone<10)
				sprintf(szBuffer, "+0%d00", timezone);
			else
				sprintf(szBuffer, "+%d00", timezone);
		}
		else
		{
			if(timezone>-10)
				sprintf(szBuffer, "-0%d00", abs(timezone));
			else
				sprintf(szBuffer, "%d00", timezone);
		}
//		cJSON_AddNumberToObject(fmt,"time_zone", tmp);
		cJSON_AddItemToObject(list, "time_zone", cJSON_CreateString(szBuffer));
		p += 2;
//		printf("file:%s line:%d\n",__FILE__, __LINE__);

		//时间戳
		utc_time = (Data_p->data[p]<<24)
				+(Data_p->data[p+1]<<16)
				+(Data_p->data[p+2]<<8)
				+Data_p->data[p+3];
		tm_p = gmtime((const time_t *)(&utc_time));
//		memset(buf, 0, 100);
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d.000 ", tm_p->tm_year+1900, tm_p->tm_mon, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
		strcat(buf,szBuffer);
		cJSON_AddStringToObject (list,"measure_time", (char *)buf);
//		printf("file:%s line:%d\n",__FILE__, __LINE__);

		//高压
		p += 4;
		tmp = (Data_p->data[p]<<8)|Data_p->data[p+1];
		cJSON_AddNumberToObject(list,"tall_pressure", tmp);

		//低压
		p += 2;
		tmp = (Data_p->data[p]<<8)|Data_p->data[p+1];
		cJSON_AddNumberToObject(list,"low_pressure", tmp);

		//心率
		p += 2;
		tmp = (Data_p->data[p]<<8)|Data_p->data[p+1];
		cJSON_AddNumberToObject(list,"heart_rate", tmp);

		//设备序列号
		sprintf( buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned   char)system_info.mac[0],
				(unsigned   char)system_info.mac[1],
				(unsigned   char)system_info.mac[2],
				(unsigned   char)system_info.mac[3],
				(unsigned   char)system_info.mac[4],
				(unsigned   char)system_info.mac[5]);
		cJSON_AddStringToObject(list,"device_serial", (char *)buf);

		char *rendered=cJSON_Print(root);
		printf("%s\n",(char *)rendered);
		cJSON_Delete(root);
		post_record_Url(root, rendered);
}
__u8 const http_up_data_rsp_argvlist[]={1,0};


static uart_cmd_list_t const uart_cmd_list[]=
{
	{RSP_POWERON,       0,              		poweron_rsp,       		poweron_rsp_argvlist},
	{CMD_QUE_STA,       ASK_QUE_STA,       que_sta_rsp,        	que_sta_rsp_argvlist},
	{CMD_TIME_SYNC,     ASK_TIME_SYNC,     time_sync_rsp,       time_sync_rsp_argvlist},
	{CMD_POWERON,       ASK_POWERON,       drv_poweron_rsp,       drv_poweron_rsp_argvlist},

	{RSP_HID_INSERT,    0,              		hid_insert_rsp,    		hid_insert_rsp_argvlist},
	{RSP_HID_PULLOUT,   0,             			hid_pullout_rsp,   		hid_pullout_rsp_argvlist},
	{RSP_HID_DATA,      0,             			hid_data_rsp,      		hid_data_rsp_argvlist},
	{RSP_PRINT_STA,     0,             			print_sta_rsp,      	print_sta_rsp_argvlist},
	{CMD_PRINT,         ASK_PRINT,     			print_ask,      				print_ask_argvlist},
	{CMD_HTTP_UP_DATA,  0,  										http_up_data_rsp,   	http_up_data_rsp_argvlist},
	{0,0,NULL}
};

static PARAM_STR param={0,0,{NULL}};


__u8 check_sum_cal(void *p, __u16 len)
{
	__u8 check_sum = 0;
	__u32 tmp = 0;
	__u16 i;
	__u8* data = p;
	for(i=0; i<len; i++)
	{
		tmp += data[i];
	}
	check_sum = (__u8)tmp;
	return check_sum;
}

__u32 procotol_check(uart_cmd_format_t* procotol_data)
{
	__u8 check_value;
	__u16 data_len;

	data_len = (procotol_data->header.length_msb<<8) + procotol_data->header.length_lsb;

	if(data_len < 4)return false;

	check_value = check_sum_cal(&(procotol_data->header.sta_param), data_len);

	printf("data_len = %d, check_value = %x\n",data_len,check_value);
	if(procotol_data->data[data_len-4] != check_value)
	{
		return false;
	}
	return true;
}

void ArgsParam(__u8 *param_line, __u8 const *argvlist, PARAM_STR* pParam)
{
	pParam->argc=0;

	if (argvlist==NULL) return;

	if(*argvlist)
		pParam->argv[pParam->argc++] = param_line;
	else
		return;


	while(*(argvlist+1))
	{
		param_line += (*argvlist);
		pParam->argv[pParam->argc] = param_line;
		argvlist++;
		if(++pParam->argc >= ARGV_MAX_NUM)
			break;
	}
}


void uart_protocol_parse(void* data_buf)
{
	const uart_cmd_list_t *list;
	struct Buffer *p_buf;
//	struct message msg;
	struct list_head *p;
	uart_send_format_t *parray;

	Data_p = (uart_cmd_format_t*)data_buf;
//	printf("file:%s line:%d\n",__FILE__, __LINE__);
	//计算校验
	if(procotol_check(Data_p) == false)
	{
		return;
	}
//	printf("file:%s line:%d\n",__FILE__, __LINE__);
	for(list=uart_cmd_list; list->callback!=NULL; list++)
	{
		if(Data_p->header.cmd_id == list->cmdword)
		{
//			printf("file:%s line:%d\n",__FILE__, __LINE__);
			Respdatalen = 0;
			ArgsParam(Data_p->data,list->argvlist,&param);
			if(list->ackword)
			{
				p_buf = (void*)buf_factory_produce(g_uart_send_buf);
				if(p_buf == NULL)
				{
					return;
				}
				sendpdata = (uart_send_format_t*)(p_buf->memory);
				sendpdata->buf = p_buf;
				Respdata = &(sendpdata->cmd);

				Respdata->header.head = 0xD0;
				Respdata->header.sta_param = 0;
				Respdata->header.sn = Data_p->header.sn;
				Respdata->header.pdt_type = Data_p->header.pdt_type;
				Respdata->header.cmd_id = list->ackword;
			}

			list->callback();

			if(list->ackword)
			{
				Respdatalen += 4;
				setresplen(Respdatalen);
				Respdata->data[Respdatalen - 4] = check_sum_cal(&(Respdata->header.sta_param), Respdatalen);

				sendpdata->poll.type = 0;
				sendpdata->list.owner = sendpdata;

				pthread_mutex_lock(&uart_poll_mutex);
				list_add_tail(&(sendpdata->list), &g_uart_send_list);
				pthread_mutex_unlock(&uart_poll_mutex);
			}

			else
			{
				pthread_mutex_lock(&uart_poll_mutex);
				p = g_uart_send_list.next;
				while(p != &g_uart_send_list)
				{
					parray = (uart_send_format_t *)(p->owner);

					if(Data_p->header.sn == parray->cmd.header.sn)
					{
						parray->poll.free_flag = 1;
						break;
					}
					p = p->next;
				}
				pthread_mutex_unlock(&uart_poll_mutex);
			}

			break;
		}
	}
}
