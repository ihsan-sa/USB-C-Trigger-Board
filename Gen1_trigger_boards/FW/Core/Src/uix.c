/*
 * uix.c
 *
 *  Created on: Aug 29, 2025
 *      Author: ihsan
 */

#include "uix.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>

extern UART_HandleTypeDef huart1;


int get_user_input(){
	uint8_t buffer[8];
	uint8_t uart_buff;
	int idx = 0;
	uart_buff = '\0';
	while(uart_buff != '\r' && uart_buff != '\n'){
		if(uart_buff != '\0'){
			buffer[idx] = uart_buff;
			idx++;
		}
		uart_buff = '\0';
		HAL_UART_Receive(&huart1, &uart_buff, 1, HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart1, &uart_buff, 1, HAL_MAX_DELAY);
	}
	buffer[idx] = '\0';
//	printf("current: %d  %s\r\n", atoi(buffer), buffer);
	return atoi(buffer);
}
