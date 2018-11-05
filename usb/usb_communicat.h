/*
 * usb_communicat.h
 *
 *  Created on: 2017年2月27日
 *      Author: work
 */

#ifndef USB_COMMUNICAT_H_
#define USB_COMMUNICAT_H_

#include <linux/types.h>

struct key_word_format_t
{
	__u8 head;
	__u8 code[7];
};

typedef struct
{
	struct Buffer *buf;
	struct key_word_format_t key_word;
}hid_format_t;

void hid_context_init(void);

#endif /* USB_COMMUNICAT_H_ */
