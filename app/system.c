/*
 * system.c
 *
 *  Created on: 2017年6月20日
 *      Author: work
 */

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <string.h>
#include <linux/types.h>
#include <time.h>
#include <uci.h>
#include <stdlib.h>

#include "net.h"
#include "http_protocol.h"
#include "key_file.h"

#include "system.h"
#include "uart_protocol.h"

extern system_info_t  system_info;
net_state_t  net_state[4];
char g_domain[128];

int getSystemTime()
{
	time_t timer;

	struct tm* t_tm;
	time(&timer);
	t_tm = localtime(&timer);
	printf("today is %4d-%02d-%02d %02d:%02d:%02d %s\n", t_tm->tm_year+1900,
	t_tm->tm_mon+1, t_tm->tm_mday, t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec, t_tm->tm_zone);
	printf("%s\n",asctime(t_tm));
	return 0;
}

__u32 get_sys_mac()
{
	struct   ifreq   ifreq;
	int   sock;
	if((sock=socket(AF_INET,SOCK_STREAM,0)) <0)
	{
		perror( "socket ");
		return   2;
	}
	strcpy(ifreq.ifr_name,"apcli0");
	if(ioctl(sock,SIOCGIFHWADDR,&ifreq) <0)
	{
		perror( "ioctl ");
		return   3;
	}

	memcpy(system_info.mac,ifreq.ifr_hwaddr.sa_data, 6);

//	printf( "%02x:%02x:%02x:%02x:%02x:%02x\n ",
//			(unsigned   char)ifreq.ifr_hwaddr.sa_data[0],
//			(unsigned   char)ifreq.ifr_hwaddr.sa_data[1],
//			(unsigned   char)ifreq.ifr_hwaddr.sa_data[2],
//			(unsigned   char)ifreq.ifr_hwaddr.sa_data[3],
//			(unsigned   char)ifreq.ifr_hwaddr.sa_data[4],
//			(unsigned   char)ifreq.ifr_hwaddr.sa_data[5]);
	return 0;
}

void get_domain_name()
{
	__s32 ret;

//	memset(g_domain,0,128);
	ret = key_file_open("/home/root/system.conf");
	if (ret == -1)
	{
		sprintf(g_domain,"120.92.2.162");
		return;
//		perror( "system.conf");
//		exit(-1);
	}
	key_file_get_string(g_domain, "cloud_set",	"domain", "120.92.2.162");
//	printf("g_domain");
//	printf(g_domain);
//	printf("\n");
	key_file_close();
}

void sys_context_init()
{
	get_domain_name();//必须在get_time_Url()之前调用
	get_sys_mac();
	get_time_Url();


	//初始化相关变量
	system_info.poweron = 0;

	memset(net_state, 0, sizeof(net_state));
	net_state[0].state = 0;
	net_state[1].state = 1;
	net_state[2].state = 2;
	net_state[3].state = 3;
	net_state[3].err = 1;//默认上电server不在线

}
