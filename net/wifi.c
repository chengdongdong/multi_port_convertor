/*
 * wifi.c
 *
 *  Created on: 2017年7月4日
 *      Author: work
 */
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <regex.h>
#include <linux/types.h>

static char * substr(const char * str, unsigned start, unsigned end)
{
	unsigned n = end - start;
	static char strbuf[256];
	memset(strbuf, '\0', sizeof(strbuf));
	strncpy(strbuf, str + start, n);
	strbuf[n] = '\0';
	return strbuf;
}

__s32 sesrch_AP(char *ssid)
{
	static regmatch_t match[10];
	size_t nmatch = 10;
	char err[128];
	__s32 i,ret = 0;
	char pattern[] = "ESSID.*";
	FILE   *stream;
	regex_t myreg;
	char  buf[102400];
	char  *p;
	char  *ap_name;
//	printf("file:%s line:%d\n",__FILE__, __LINE__);
	memset( buf, 0, sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
	stream = popen( "iwlist apcli0 scanning", "r" );
	fread( buf, sizeof(char), sizeof(buf),  stream);  //将刚刚FILE* stream的数据流读取到buf中
	pclose( stream );
//	printf("file:%s line:%d\n",__FILE__, __LINE__);
	ret = regcomp(&myreg, pattern, REG_EXTENDED | REG_NEWLINE);
	if (ret != 0)
	{
		regerror(ret, &myreg, err, sizeof(err));
		fprintf(stderr, "%s\n",err);
		regfree(&myreg);
		return -1;
	}

	p = buf;
	i = 0;
	while(1)
	{
		ret = regexec(&myreg, p, nmatch, match, 0);
		if (ret != 0)
		{
				break;
		}
		else
		{
//			printf("file:%s line:%d\n",__FILE__, __LINE__);
			ap_name = substr(p, match[0].rm_so,match[0].rm_eo);
//			printf("file:%s line:%d\n",__FILE__, __LINE__);
//			printf(" $%d='%s'\n", i++, ap_name);
//			printf("file:%s line:%d\n",__FILE__, __LINE__);
			if(strstr(ap_name, ssid))
			{
				//找到
				regfree(&myreg);
				return 0;
			}
			p = p + match[0].rm_eo;
		}
	}

	regfree(&myreg);
	return -1;
}
