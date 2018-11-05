/*
 * msg_engine.h
 *
 *  Created on: 2015年4月23日
 *      Author: work
 */

#ifndef MSG_ENGINE_H_
#define MSG_ENGINE_H_

#include <linux/types.h>
//#include "message.h"

struct message
{
	__s32 id;			//消息队列id
	__u32 prio;		//消息优先级
	void *p;  //消息队列传递的指针
	__s32 data_len; //协议本身的长度
};


__s32 mailbox_create(char *name, __uint32_t msg_num);
__s32 mailbox_post(struct message *msg);
__s32 mailbox_pend(struct message *msg);
__s32 mailbox_timedpend(struct message *msg, struct timespec* timeout);
//int  mailbox_timedpend(struct message *msg);
__s32 mailbox_timedpost(struct message *msg, struct timespec* timeout);
void msg_cast(struct message *msg, __s32 id, __s32 prio);
#endif /* MSG_ENGINE_H_ */
