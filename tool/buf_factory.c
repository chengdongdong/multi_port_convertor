/*
 * buf_factory.c
 *
 *  Created on: 2015年5月8日
 *      Author: work
 */
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "debug.h"
#include "buf_factory.h"


//extern struct buffer_factory *g_c1_sample_buf;
//static struct buffer_factory *buf_factory;

__s32 create_buf_factory(__u32 num, __u32 size, struct buffer_factory **p)
{
	__s32 i = 0;
	__s32 j = 0;

	struct buffer_factory *buf_factory;

	buf_factory  = malloc(sizeof( struct buffer_factory));
	if(buf_factory == NULL)
		ERROR("malloc");

	init_list_head(&buf_factory->list);
	pthread_mutex_init (&buf_factory->list_mutex, NULL);

	buf_factory->buf_array = calloc(num, sizeof(struct Buffer));
	if(buf_factory->buf_array == NULL)
	{
		free(buf_factory);
		ERROR("calloc");
	}

	for(i=0;i<num;i++)
	{
		buf_factory->buf_array[i].memory = malloc(size);
		if(buf_factory->buf_array[i].memory== NULL)
		{
			for(j=i-1; j>=0; j--)
				free(buf_factory->buf_array[i].memory);
			free(buf_factory->buf_array);
			free(buf_factory);
			ERROR("malloc");
		}
		buf_factory->buf_array[i].node = malloc(sizeof(struct list_head));
		if(buf_factory->buf_array[i].node == NULL)
		{
			for(j=i-1; j>=0; j--)
				free(buf_factory->buf_array[i].node);

			for(j=i; j>=0; j--)
				free(buf_factory->buf_array[i].memory);

			free(buf_factory->buf_array);
			free(buf_factory);
			ERROR("malloc");
		}
		buf_factory->buf_array[i].node->owner = &buf_factory->buf_array[i];
		list_add_tail(buf_factory->buf_array[i].node, &buf_factory->list);
	}

	*p = buf_factory;

	return 0;
}

struct Buffer *buf_factory_produce(struct buffer_factory *buf_factory)
{
	struct list_head *node_of_buf;
	struct Buffer *buf;

	pthread_mutex_lock(&buf_factory->list_mutex);

	if(list_empty(&buf_factory->list))
		ERROR("buf factory crash !");

	node_of_buf = buf_factory->list.next;
	buf = (struct Buffer *)(node_of_buf->owner);
	list_del(node_of_buf);

	pthread_mutex_unlock(&buf_factory->list_mutex);

	return buf;
}

void buf_factory_recycle(__s32 arg, void  *__buf__ , struct buffer_factory *buf_factory)
{
	struct Buffer *buf;

	buf = (struct Buffer*)__buf__;

	pthread_mutex_lock(&buf_factory->list_mutex);
	list_add_tail(buf->node, &buf_factory->list);
	pthread_mutex_unlock(&buf_factory->list_mutex);
}
