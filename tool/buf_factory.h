/*
 * buf_factory.h
 *
 *  Created on: 2015年5月8日
 *      Author: work
 */

#ifndef TOOLS_BUF_FACTORY_H_
#define TOOLS_BUF_FACTORY_H_

#include "list.h"
#include <linux/types.h>
#include <thread.h>

struct Buffer
{
	void *memory;
	struct list_head *node;
};

struct buffer_factory
{
	struct Buffer *buf_array;
	struct list_head list;
	pthread_mutex_t list_mutex;
};

__s32 create_buf_factory(__u32 num, __u32 size, struct buffer_factory **p);

struct Buffer *buf_factory_produce(struct buffer_factory *g_buf_factory);
void buf_factory_recycle(__s32 arg, void *buf,struct buffer_factory *g_buf_factory);

#endif /* TOOLS_BUF_FACTORY_H_ */
