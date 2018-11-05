/*
 * usb.h
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */

#ifndef USB_H_
#define USB_H_

#include <linux/types.h>


__s32 open_hid();
void set_hid(__s16 fd, __s16* usb_product);

#endif /* USB_H_ */
