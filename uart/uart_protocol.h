/*
 * uart_protocol.h
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */

#ifndef UART_PROTOCOL_H_
#define UART_PROTOCOL_H_

#include <linux/types.h>

//NTF——主动上传的命令
//RSP——收到的响应

//CMD——收到的命令
//ASK——应答的命令


//模块控制命令
#define NTF_POWERON          0x10
#define RSP_POWERON          0x10

#define CMD_QUE_STA           0x11
#define ASK_QUE_STA           0x11

#define CMD_TIME_SYNC           0x12
#define ASK_TIME_SYNC           0x12

#define CMD_POWERON          0x13
#define ASK_POWERON          0x13


#define NTF_ERR_STA          0x1F


//USB设备数据传输命令
#define NTF_HID_INSERT       0x20
#define RSP_HID_INSERT       0x20

#define NTF_HID_PULLOUT      0x21
#define RSP_HID_PULLOUT      0x21

#define NTF_HID_DATA         0x22
#define RSP_HID_DATA         0x22

#define CMD_PRINT            0x23
#define ASK_PRINT            0x23

#define NTF_PRINT_STA        0x24
#define RSP_PRINT_STA        0x24


//服务器透传服务命令
#define CMD_HTTP_UP_DATA     0x31
#define NTF_HTTP_UP_DATA     0x31

typedef struct
{
	__u8  cmdword;
	__u8  ackword;
	void    (*callback)(void);
	__u8   const *argvlist;
}uart_cmd_list_t;

#define ARGV_MAX_NUM	24
typedef struct
{
	__u32 	len;
	__u8    argc;
	__u8    *argv[ARGV_MAX_NUM];
} PARAM_STR;

typedef struct
{
	__u8 	 cmd_id;       //命令字，具体定义见扩展协议2.3
	__u32  len;          //所传数据长度，如果没有数据则赋值0
	__u8   pdt_type;     //产品类型，血压计以此进行协议分层解析
	__u8   data[128];    //所传数据
	__u8   poll_type;
}uart_send_info_t;



void poweron_ntf(__u8 state);
void err_sta_ntf(__u8 state, __u8 course, __u8 err);
void hid_insert_ntf(__u8 dev_type);
void hid_pullout_ntf(__u8 dev_type);
void hid_data_ntf(__u8 *data, __u32 length, __u8 dev_type);
//void print_sta_ntf(__u8 state);
void http_up_data_ntf(__u8 state);
void uart_protocol_parse(void* data_buf);

#endif /* UART_PROTOCOL_H_ */
