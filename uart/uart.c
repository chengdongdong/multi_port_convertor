/*
 * uart.c
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */


#include <stdio.h>
#include <string.h>
#include <linux/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>


__s32 set_opt(__s32 fd, __s32 nSpeed, __s32 nBits, char nEvent, __s32 nStop)
{
	struct termios newtio,oldtio;
	if(tcgetattr( fd,&oldtio)  !=  0)
    {
		perror("SetupSerialOpt -1");
		return -1;
    }
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

	switch( nEvent )
    {
    case 'O':                     //奇校验
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':                     //偶校验
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':                    //无校验
        newtio.c_cflag &= ~PARENB;
        break;
    }

	switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }

	if( nStop == 1 )
    {
		newtio.c_cflag &=  ~CSTOPB;
    }
	else if ( nStop == 2 )
    {
		newtio.c_cflag |=  CSTOPB;
    }

	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 1;//每次就读一个字节

	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
		perror("uart set error");
		return -1;
    }

	printf("set uart done!\n");
	return 0;
}

__s32 open_port(void)
{
	__s32 fd = 0;

	fd = open( "/dev/ttyS1", O_RDWR&~O_NOCTTY);
//	fd = open( "/dev/ttyUSB0", O_RDWR&~O_NOCTTY);
	if (-1 == fd)
	{
		perror("Can't Open Serial Port\n");
		return -1;
	}

	if(fcntl(fd, F_SETFL, 0) < 0)
	{
		perror("fcntl failed!\n");
		return -1;
	}

//	if(isatty(STDIN_FILENO) == 0)
//	{
//		perror("standard input is not a terminal device\n");
//		return -1;
//	}

	printf("uart_fd = %d\n",fd);
	return fd;
}

