/*
 * uart.h
 *
 *  Created on: 2017年2月24日
 *      Author: work
 */

#ifndef UART_H_
#define UART_H_

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
int open_port(void);

#endif /* UART_H_ */



