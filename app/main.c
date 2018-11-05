/*
 * main.c
 *
 *  Created on: 2017年2月20日
 *      Author: work
 */

#include <unistd.h>
#include <stdlib.h>
//#include <signal.h>
//#include <string.h>
//#include <regex.h>

#include "uart_communicat.h"
#include "uart_protocol.h"
#include "http_protocol.h"
#include "usb_communicat.h"
//#include "thread.h"
//#include "mailbox.h"
#include "net.h"
#include "system.h"
//#include "wifi.h"
#include "udisk.h"

system_info_t  system_info =
{
	.firmware_ver = {0x01, 0x00, 0x00},
	.hardware_ver = {0x01, 0x00, 0x00},
	.protocol_ver = {0x01, 0x00, 0x00},
	.time_valid = 0,
	.poweron = 1 //上电未完成初始化
};


__s32 g_mbx_id= 0;
int main(void)
{
	__u32 timing_interval = 0;
	sys_context_init();//必须先调用
	uart_context_init();
	hid_context_init();
	net_context_init();
	udisk_context_init();

	sleep(1);
	poweron_ntf(0);

	while(1)
	{
		sleep(1);
		timing_interval++;
		if(timing_interval>=60)
		{
			timing_interval = 0;
			get_time_Url();
		}
//		getSystemTime();
	}
	return 0;
}

