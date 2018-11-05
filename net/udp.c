/*
 * udp.c
 *
 *  Created on: 2017年7月19日
 *      Author: work
 */


#include <arpa/inet.h>
#include <linux/if.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "debug.h"

__s32 g_udp_fd;


__s32 udp_socket_create()
{
	__s32 opt = 1;
	struct timeval tv_out;

	if ((g_udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
		ERROR("socket");
    }

    //设置该套接字为广播类型，
	if(setsockopt(g_udp_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)) == -1)
    {
		printf("setsockopt error!\n");
    }

	// 设置超时
	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;
	setsockopt(g_udp_fd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));

	return 0;
}

__s32 check_dhcp_state(const char* ifname)
{
	struct   sockaddr_in *sin;
	struct   ifreq ifr_ip;

	memset(&ifr_ip, 0, sizeof(ifr_ip));
	strncpy(ifr_ip.ifr_name, ifname, sizeof(ifr_ip.ifr_name) - 1);
	if( ioctl( g_udp_fd, SIOCGIFADDR, &ifr_ip) < 0 )
	{
//		MSG("Get local IP error!");
		return -1;
	}

	sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;

//	MSG("local ip is %s. \n",inet_ntoa(sin->sin_addr));

	return 0;
}
