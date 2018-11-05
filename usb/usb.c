/*
 * usb.c
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */

#include <fcntl.h>
#include <linux/hidraw.h>
#include <linux/types.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static char *possible_hid[] = {"hidraw0","hidraw1","hidraw2"};


static __s32 do_check(const char* relative_file_name)
{
	__s32 ret = 0;
	char absolute_file_name[32] = "/dev/";
	strcat(absolute_file_name, relative_file_name);
	ret = access(absolute_file_name, F_OK);
	return ret;
}

__s32 open_hid()
{
	__s32 fd = 0;
	__s32 ret;
	__u32 i;
	char absolute_file_name[32] = "/dev/";
	for(i = 0; i < 3; i++)
	{
		ret = do_check(possible_hid[i]);
		if (ret == 0)
		{
			strcat(absolute_file_name, possible_hid[i]);
			fd = open(absolute_file_name, O_RDONLY);
			if (fd < 0)
			{
				perror("Unable to open device");
//				return -1;
			}
			else
			{
				break;
			}
		}
	}

	return fd;
}


static const char *bus_str(int bus)
{
	switch (bus)
	{
		case BUS_USB:
			return "USB";
			break;

		case BUS_HIL:
			return "HIL";
			break;

		case BUS_BLUETOOTH:
			return "Bluetooth";
			break;

		case BUS_VIRTUAL:
			return "Virtual";
			break;

		default:
			return "Other";
			break;
	}
	return "Other";
}



void set_hid(__s16 fd, __s16* usb_product)
{
//	int i;
	int res;
//	int desc_size = 0;
	char buf[512];
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

//	/* Get Report Descriptor Size */
//	res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
//	if (res < 0)
//		perror("HIDIOCGRDESCSIZE");
//	else
//		printf("Report Descriptor Size: %d\n", desc_size);
//
//	/* Get Report Descriptor */
//	rpt_desc.size = desc_size;
//	res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
//	if (res < 0)
//	{
//		perror("HIDIOCGRDESC");
//	}
//	else
//	{
//		printf("Report Descriptor:\n");
//		for (i = 0; i < rpt_desc.size; i++)
//			printf("%hhx ", rpt_desc.value[i]);
//		puts("\n");
//	}

	/* Get Raw Name */
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWNAME");
	else
		printf("Raw Name: %s\n", buf);

	/* Get Physical Location */
	res = ioctl(fd, HIDIOCGRAWPHYS(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWPHYS");
	else
		printf("Raw Phys: %s\n", buf);

	/* Get Raw Info */
	res = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (res < 0)
	{
		perror("HIDIOCGRAWINFO");
	}
	else
	{
		printf("Raw Info:\n");
		printf("\tbustype: %d (%s)\n",
			info.bustype, bus_str(info.bustype));
		printf("\tvendor: 0x%04hx\n", info.vendor);
		printf("\tproduct: 0x%04hx\n", info.product);

		*usb_product = info.product;
	}
}




