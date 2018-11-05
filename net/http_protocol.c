/*
 * http_protocol.c
 *
 *  Created on: 2017年3月13日
 *      Author: work
 */

//#define _XOPEN_SOURCE

#include <stdio.h>
#include <curl/curl.h>
#include <linux/types.h>
#include <stdlib.h>
#include <time.h>


#include "cJSON.h"
#include "uart_protocol.h"
#include "system.h"
#include "net.h"

char* filename = "/mnt/targets/test.bin";
extern system_info_t  system_info;
extern net_state_t  net_state[4];
extern char g_domain[128];

static __u32 heart_err_time = 0;

//void cjson_test()
//{
//	cJSON *root,*fmt;
//	root = cJSON_CreateObject();
//	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
//	cJSON_AddItemToObject(root, "format", fmt=cJSON_CreateObject());
//	cJSON_AddStringToObject(fmt,"type",     "rect");
//	cJSON_AddNumberToObject(fmt,"width",        1920);
//	cJSON_AddNumberToObject(fmt,"height",       1080);
//	cJSON_AddFalseToObject (fmt,"interlace");
//	cJSON_AddNumberToObject(fmt,"frame rate",   24);
//	char *rendered=cJSON_Print(root);
//	postUrl(rendered);
//}



/*************************************
回调函数，处理post返回数据的地方
*************************************/
static size_t get_time_parse(void* buffer, size_t size, size_t nmemb, char * useless)
{
	struct tm t_tm;
	time_t timep;
	struct timeval tv;
	char *sub;
	__u32 msec;

	printf("%s\n",(char *)buffer);
	cJSON *root = cJSON_Parse(buffer);
	if (!root)
	{
		printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
		return 0;
	}

	cJSON *errno = cJSON_GetObjectItem(root, "errno");
	if (!errno)
	{
		//cJSON_GetErrorPtr(),not thread safe,the return_parse_end parameter of cJSON_ParseWithOpts can be used instead
		//也就是cJSON_Parse和cJSON_GetErrorPtr,不能并行调用，但是本工程的程序结构没有问题
		printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return 0;
	}

	if((errno->type == cJSON_Number)&&(errno->valueint == 0))
	{
		cJSON *data = cJSON_GetObjectItem(root, "data");
		if (!data)
		{
			printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
			cJSON_Delete(root);
			return 0;
		}

		cJSON *time = cJSON_GetObjectItem(data, "time");
		if (!time)
		{
			printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
			cJSON_Delete(root);
			return 0;
		}

		sub = cJSON_Print(time);
		printf("%s\n", sub);

		sscanf(sub, "\"%04d-%02d-%02d %02d:%02d:%02d.%03d\"",
				&(t_tm.tm_year),
				&(t_tm.tm_mon),
				&(t_tm.tm_mday ),
				&(t_tm.tm_hour),
				&(t_tm.tm_min),
				&(t_tm.tm_sec),
				&(msec));
		tv.tv_usec = msec * 1000;
		free(sub);

		printf("web time = %d-%d-%d %d:%d:%d\n", t_tm.tm_year,t_tm.tm_mon,t_tm.tm_mday,t_tm.tm_hour,t_tm.tm_min ,t_tm.tm_sec);
		t_tm.tm_year -= 1900;
		timep = mktime(&t_tm);
		tv.tv_sec = timep;
		settimeofday (&tv, (struct timezone *) 0);

		cJSON *time_zone = cJSON_GetObjectItem(data, "time_zone");
		if (!time_zone)
		{
			printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
			cJSON_Delete(root);
			return 0;
		}

		if(time_zone->type == cJSON_String)
		{
			system_info.timezone = atoi(time_zone->valuestring)/50;
			printf("system_info.timezone = %d",system_info.timezone);
			system_info.time_valid = 1;
		}
		else
		{
			printf("Cjson Type Error!\n");
		}
	}
	else
	{
		printf("Cjson Type Error!\n");
	}

	cJSON_Delete(root);

	return size*nmemb;
}


__u32 get_time_Url()
{
	CURL *curl;
	CURLcode op_code;
	char url[256];
	__u32 n = 0;

	curl = curl_easy_init();    // 初始化
	if (curl)
    {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_time_parse); //对返回的数据进行操作的函数地址
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL); //这是write_data的第四个参数值
//		curl_easy_setopt(curl, CURLOPT_URL,"http://chronos.ihealthlabs.com.cn/dataservice/blood_pressure/get_time");
		n += sprintf(url+n, "http://");
		n += sprintf(url+n, g_domain);
		n += sprintf(url+n, "/dataservice/blood_pressure/get_time");
		curl_easy_setopt(curl, CURLOPT_URL, url);   // 指定url
//		curl_easy_setopt(curl, CURLOPT_URL,"http://120.92.2.162/dataservice/blood_pressure/get_time");
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
		op_code = curl_easy_perform(curl);   // 执行
		printf("op_code = %d\n", op_code);
		if(op_code != 0)
		{
			heart_err_time++;
			if(heart_err_time>=3)
			{
				heart_err_time = 0;
				net_state[3].err = 1;//不在线
			}
		}
		else
		{
			heart_err_time = 0;
			net_state[3].err = 0;//在线
		}
		curl_easy_cleanup(curl);
		return 1;
    }
	return 0;
}



/*************************************
回调函数，处理post返回数据的地方
*************************************/
static size_t post_record_parse(void* buffer, size_t size, size_t nmemb, char * useless)
{
	printf("%s\n",(char *)buffer);
	cJSON *root = cJSON_Parse(buffer);
	if (!root)
	{
		printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
		return 0;
	}

	cJSON *errno = cJSON_GetObjectItem(root, "errno");
	if (!errno)
	{
		printf("Cjson Error because: [%s]\n",cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return 0;
	}

	if(errno->type == cJSON_Number)
	{
		http_up_data_ntf(errno->valueint);
	}
	else
	{
		printf("Cjson Type Error!\n");
	}

	cJSON_Delete(root);
	return size*nmemb;
}

__u32 post_record_Url(cJSON *root, char *p)
{
	CURL *curl;
	CURLcode op_code;
	char url[256];
	__u32 n = 0;

	curl = curl_easy_init();

	if (curl)
    {
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept:application/json");
		headers = curl_slist_append(headers, "Content-Type:application/json;charset=utf-8");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, p);    // 指定post内容

		n += sprintf(url+n, "http://");
		n += sprintf(url+n, g_domain);
		n += sprintf(url+n, "/dataservice/blood_pressure/add");
		curl_easy_setopt(curl, CURLOPT_URL, url);   // 指定url
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, post_record_parse); //对返回的数据进行操作的函数地址
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL); //这是write_data的第四个参数值
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

		op_code = curl_easy_perform(curl);
		printf("op_code = %d\n", op_code);
		if(op_code != 0)
		{
			op_code = curl_easy_perform(curl);
			printf("op_code = %d\n", op_code);
			if(op_code != 0)
			{
				op_code = curl_easy_perform(curl);
				printf("op_code = %d\n", op_code);
				net_state[3].err = 1;//不在线
			}
		}

		if(op_code == 0)
		{
			net_state[3].err = 0;//在线
//			http_up_data_ntf(0);//正常执行，回调函数发送串口命令
		}
		else
		{
			http_up_data_ntf(2);
		}

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
    }
	free(p);
	return 0;
}





