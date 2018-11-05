/*
 * udisk.c
 *
 *  Created on: 2015年8月19日
 *      Author: work
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <fcntl.h>
//#include <regex.h>
//#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
//#include <linux/netlink.h>
#include <sys/mount.h>
#include <sys/statfs.h>
//#include <semaphore.h>
#include <sys/stat.h>

#include "thread.h"
#include "uart_protocol.h"
#include "udisk.h"
#include "net.h"
#include "key_file.h"
#include "wifi.h"

#define UEVENT_BUFFER_SIZE 2048

__u32 udisk_num = 0;

struct UDISK g_udisk[3];
extern net_state_t  net_state[4];


void read_wifi_param(struct UDISK *udisk)
{
	__s32 ret;
	char key_file_name[32] = {0};
	char ssid[32];
	char passwd[32];
	char cmd[128];
//	char buf[256]; //实际测量管道返回长度38 ，因此上256肯定够
	__u32 n = 0;
//	FILE   *stream;

	/* 打开系统配置文件 */

	strcat(key_file_name, (char *)udisk->dir);
	strcat(key_file_name, "/system.conf");
	printf("%s\n", key_file_name);
	ret = key_file_open(key_file_name);
	if (ret == -1)
	{
		printf("key_file_open error!!!\n");
		err_sta_ntf(1, 0, 2);
		net_state[1].course = 0;
		net_state[1].err = 2;
		return;
	}

	key_file_get_string(ssid, "wifi_set",	"ssid", "skywalker");
	key_file_get_string(passwd, "wifi_set",	"passwd", "wireless");

	//	如果是出厂时就设置wifi，下边的if else判断显然不合适
//	if(sesrch_AP(ssid) == -1)
//	{
//		printf("cant find ap error!!!\n");
//		err_sta_ntf(1, 0, 3);
//		net_state[1].course = 0;
//		net_state[1].err = 3;
//	}
//	else
	{
		n = 0;
		n += sprintf(cmd+n, "uci set wireless.sta.ssid='");
		n += sprintf(cmd+n, ssid);
		n += sprintf(cmd+n, "'");
		printf("%s\n", cmd);
		system(cmd);
//		system("uci commit wireless");

		n = 0;
		n += sprintf(cmd+n, "uci set wireless.sta.key='");
		n += sprintf(cmd+n, passwd);
		n += sprintf(cmd+n, "'");
		system(cmd);
		system("uci commit wireless");

		system("/etc/init.d/network restart");
//		n += sprintf(cmd+n, passwd);
//		stream = popen(cmd, "r" );
//		fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
//		pclose( stream );
	}

	//拷贝出config文件
	n = 0;
	n += sprintf(cmd+n, "cp ");
	n += sprintf(cmd+n, key_file_name);
	n += sprintf(cmd+n, " ");
	n += sprintf(cmd+n, "/home/root");
	system(cmd);
//	stream = popen(cmd, "r" );
//	fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
//	pclose( stream );

	key_file_close();
}


__u32 GetStorageInfo(char * MountPoint, __u32 *AllCapacity, __u32 *LeftCapacity)
{
	struct statfs statFS;

	__u64 freeBytes = 0;
	__u64 totalBytes = 0;

	if (statfs(MountPoint, &statFS) == -1)
	{   //获取分区的状态
		printf("statfs failed for path->[%s]\n", MountPoint);
		return (-1);
	}

	totalBytes = (__u64 ) statFS.f_blocks * (__u64 ) statFS.f_frsize; //以字节为单位的总容量
	freeBytes = (__u64 ) statFS.f_bfree * (__u64 ) statFS.f_frsize; //以字节为单位的剩余容量

	*AllCapacity = totalBytes >> 10; //以KB为单位的总容量
	*LeftCapacity = freeBytes >> 10; //以KB为单位的剩余容量

	return 0;
}


struct UDISK * do_mount(const char* relative_file_name)
{
	__s32 ret = 0;
	__u32 all_capacity;
	__u32 left_capacity;
	struct UDISK *udisk = NULL;

	char absolute_file_name[16] = "/dev/";
	char mount_point[16] = "/mnt/";

	strcat(absolute_file_name, relative_file_name);
	strcat(mount_point, relative_file_name);

	ret = mount(absolute_file_name, mount_point, "vfat", MS_NOSUID, "iocharset=utf8");
	if (ret == 0)
	{
		GetStorageInfo(mount_point, &all_capacity, &left_capacity);
//		n += sprintf(buf+n,"\n %s is inserted.\n", absolute_file_name);
//		n += sprintf(buf+n,"\n All Capacity:%uKB\n", all_capacity);
//		n += sprintf(buf+n,"\n Left Capacity:%uKB\n", left_capacity);

		if(strstr(mount_point, "sda1"))
		{
			udisk = &g_udisk[0];
		}
		else if(strstr(mount_point, "sdb1"))
		{
			udisk = &g_udisk[1];
		}
		else if(strstr(mount_point, "sdc1"))
		{
			udisk = &g_udisk[2];
		}

		memset(udisk->dir,0,32);
		strcpy((char*)udisk->dir, mount_point);
		printf("udisk->dir %s\n",udisk->dir );

		udisk->all_capacity = all_capacity;
		udisk->left_capacity = left_capacity;
		udisk->state = 0;
		udisk->port = 0;

	}
// 不能因为插入坏U盘程序直接退出，因此注释掉下面两句
//	else if (ret != 0)
//		perror("mount");

	return udisk;
}

struct UDISK * do_umount(const char* relative_file_name)
{
	struct UDISK *udisk;
	char mount_point[16] = "/mnt/";
	strcat(mount_point, relative_file_name);
	umount(mount_point);

	if(strstr(mount_point, "sda1"))
	{
		udisk = &g_udisk[0];
	}
	else if(strstr(mount_point, "sdb1"))
	{
		udisk = &g_udisk[1];
	}
	else if(strstr(mount_point, "sdc1"))
	{
		udisk = &g_udisk[2];
	}
	udisk->state = 1;
	return udisk;
}

__s32 do_check(const char* relative_file_name)
{
	__s32 ret = 0;
	char absolute_file_name[16] = "/dev/";
	strcat(absolute_file_name, relative_file_name);
	ret = access(absolute_file_name, F_OK);
	return ret;
}

void udisk_tsk(void)
{
	char *possible_udisk[] = {"sda1","sdb1","sdc1"};
	__s32 i,ret = 0;
	struct UDISK *udisk;

	while(1)
	{
		sleep(1);
		/* 检测程序启动前是否已有u盘接入 */
		for(i = 0; i < 3; i++)
		{
			ret = do_check(possible_udisk[i]);
			if (ret == 0)
			{
				//检查udisk挂载目录
				char mount_file_name[16] = "/mnt/";
				strcat(mount_file_name, possible_udisk[i]);
				ret = access(mount_file_name, F_OK);
				if(ret != 0)
				{
					mkdir(mount_file_name, S_IRWXU);
				}

				udisk = do_mount(possible_udisk[i]);

				if(udisk)
				{
					hid_insert_ntf(5); //插入U盘
					read_wifi_param(udisk);
					printf("udisk_insert_ntf!\n");
				}
				else // 防止出现那种只能显示，但是挂不上的坏U盘
					continue;

				while(!do_check(possible_udisk[i]))
				{
					usleep(1000);
				}

				udisk = do_umount(possible_udisk[i]);
				hid_pullout_ntf(5);
				printf("udisk_pullout_ntf!\n");

				break;
			}
		}
	}
}

void udisk_context_init()
{
	g_udisk[0].state = 1;
	g_udisk[1].state = 1;
	g_udisk[2].state = 1;
	add_new_thread(NULL, (void *)&udisk_tsk, 8, 160*1024);
}


