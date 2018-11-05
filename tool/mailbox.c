/*
 * msg_engine.c
 *
 *  Created on: 2015年4月23日
 *      Author: work
 */

#include <fcntl.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "debug.h"
//#include "msg_factory.h"

#include "mailbox.h"

__s32 mailbox_create(char *name, __u32 msg_num)
{
	mqd_t queue_id;
	struct mq_attr attr;

	attr.mq_flags = 0;
	attr.mq_maxmsg = msg_num;
	attr.mq_msgsize = sizeof(struct message);

	queue_id = mq_open(name, O_CREAT | O_RDWR, 0x0666, &attr);
	if (queue_id == (mqd_t) -1)
	{
		ERROR("mq_open");
	}

	return queue_id;
}

__s32  mailbox_post(struct message *msg)
{
	__s32 ret = 0;
	__s32 queue_id = msg->id;
	__s32 prio = msg->prio;

	ret = mq_send(queue_id, (char*) msg, sizeof(struct message), prio);
	if (ret == -1)
	{
		ERROR("mq_send");
	}
	return ret;
}

int  mailbox_pend(struct message *msg)
{
	__s32 ret = 0;
	__s32 queue_id = msg->id;
	__s32 prio = msg->prio;

	ret = mq_receive(queue_id, (char*) msg, sizeof(struct message), &prio);
	if (ret == -1)
	{
		ERROR("mq_receive");
	}
	return ret;
}

__s32  mailbox_timedpend(struct message *msg, struct timespec* timeout)
{
	__s32 ret = 0;
	__s32 queue_id = msg->id;
	__s32 prio = msg->prio;

	ret = mq_timedreceive(queue_id, (char*) msg, sizeof(struct message), &prio, timeout);
//	if(ret < 0)
//		perror("mq_timedreceive");

	return ret;
}


__s32  mailbox_timedpost(struct message *msg, struct timespec* timeout)
{
	__s32 ret = 0;
	__s32 queue_id = msg->id;
	__s32 prio = msg->prio;

	ret = mq_timedsend(queue_id, (char*) msg, sizeof(struct message), prio, timeout);
	if(ret < 0)
		perror("mq_timedsend");

	return ret;
}

void msg_cast(struct message *msg, __s32 id, __s32 prio)
{
	msg->id = id;
	msg->prio = prio;
}

