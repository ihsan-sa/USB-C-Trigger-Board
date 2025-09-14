/*
 * AP33772S.c
 *
 *  Created on: Aug 29, 2025
 *      Author: ihsan
 */

/* Includes */
#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "AP33772S.h"
#include "uix.h"

/* Extern */
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;


/* Struct */
typedef struct{
	int _voltage;
	int _current;
	bool _detected;
	bool _pps;
} pdo_vals;

/* Private */
void _print_arr(uint8_t *pdo_raw, int bytes);
static void _get_data(uint8_t addr, int len);
static int _get_voltage();
static int _get_current();
static int _get_temperature();
static pdo_vals _extract_PDO(uint8_t *pdo_raw);
static void _print_max_curr(uint8_t curr_val);
uint8_t pdo_voltages[13];
uint8_t pdos[13];
uint8_t pdo_idx;

void _print_max_curr(uint8_t curr_val){
	switch(curr_val){
	case 0: printf("0.00A to 1.24A"); break;
	case 1: printf("1.25A to 1.49A"); break;
	case 2: printf("1.5A to 1.74A"); break;
	case 3: printf("1.750A to 1.99A"); break;
	case 4: printf("2.00A to 2.24A"); break;
	case 5: printf("2.25A to 2.49A"); break;
	case 6: printf("2.5A to 2.74A"); break;
	case 7: printf("2.75A to 2.99A"); break;
	case 8: printf("3.00A to 3.24A"); break;
	case 9: printf("3.25A to 3.49A"); break;
	case 10: printf("3.50A to 3.74A"); break;
	case 11: printf("3.75A to 3.99A"); break;
	case 12: printf("4.00A to 4.24A"); break;
	case 13: printf("4.25A to 4.49A"); break;
	case 14: printf("4.50A to 4.99A"); break;
	case 15: printf("5.00A or higher"); break;
	}
}

void _print_curr_lim(uint8_t curr_val){
	switch(curr_val){
	case 0: printf("1.00A"); break;
	case 1: printf("1.25A"); break;
	case 2: printf("1.50A"); break;
	case 3: printf("1.75A"); break;
	case 4: printf("2.00A"); break;
	case 5: printf("2.25A"); break;
	case 6: printf("2.50A"); break;
	case 7: printf("2.75A"); break;
	case 8: printf("3.00A"); break;
	case 9: printf("3.25A"); break;
	case 10: printf("3.50A"); break;
	case 11: printf("3.75A"); break;
	case 12: printf("4.00A"); break;
	case 13: printf("4.25A"); break;
	case 14: printf("4.50A"); break;
	case 15: printf("5.00A"); break;
	}
}


void req_PDO_options() {
	uint8_t pdo_raw[2];  // buffer for all PDOs
	printf("\r\n\r\n*******************PDO OPTIONS********************\r\n");
	printf("\r\n\tSPR:\r\n\r\n");
	pdo_idx = 0;
	for(int j = 0; j < 7; j++){
	  HAL_I2C_Mem_Read(&hi2c1,
					   AP33772S_ADDR,
					   0x21 + j,             // register address for SRCPDO
					   I2C_MEMADD_SIZE_8BIT,
					   pdo_raw,
					   2,
					   HAL_MAX_DELAY);

//	  printf("PDO %d:\t", j+1);
//	  _print_arr(pdo_raw, 2);
	  pdo_vals pdo = _extract_PDO(pdo_raw);
	  if(!pdo._detected){
		  printf("PDO %d not detected.\r\n", j+1);
	  } else {
		  printf("PDO %d:\tVoltage:  %dV\tMax current: ", j+1, pdo._voltage/10);
		  _print_max_curr(pdo._current);
		  printf("\tPPS: %s\r\n", (pdo._pps) ? "Yes" : "No");
		  pdos[pdo_idx] = j + 1;
		  pdo_voltages[pdo_idx] = pdo._voltage/10;
		  pdo_idx++;
	  }
	}

	printf("\r\n\tEPR:\r\n\r\n");
		for(int j = 7; j < 13; j++){
		  HAL_I2C_Mem_Read(&hi2c1,
						   AP33772S_ADDR,
						   0x21 + j,             // register address for SRCPDO
						   I2C_MEMADD_SIZE_8BIT,
						   pdo_raw,
						   2,
						   HAL_MAX_DELAY);

//		  printf("PDO %d:\t", j+1);
//		  _print_arr(pdo_raw, 2);
		  pdo_vals pdo = _extract_PDO(pdo_raw);
		  if(!pdo._detected){
			  printf("PDO %d not detected.\r\n", j+1);
		  } else {
			  printf("PDO %d: \tVoltage: %dV\tMax current: ", j+1, pdo._voltage /5);
			  _print_max_curr(pdo._current);
			  printf("\tPPS: %s\r\n", (pdo._pps) ? "Yes" : "No");
			  pdos[pdo_idx] = j + 1;
			  pdo_voltages[pdo_idx] = pdo._voltage/5;
			  pdo_idx++;
		  }
		}
}

pdo_vals _extract_PDO(uint8_t *pdo_raw){
	pdo_vals ret;
	ret._detected = (pdo_raw[1] & (1<<7)) >> 7;
	ret._pps = (pdo_raw[1] & (1<<6)) >> 6;
	ret._current = (pdo_raw[1] & (0b00111100)) >> 2;
	ret._voltage = pdo_raw[0];

	return ret;
}

void select_PDO(uint8_t PDO, uint8_t current){
	uint8_t buff[2];
	buff[1] = (PDO << 4) | current;

	uint8_t pdo_raw[2];
	HAL_I2C_Mem_Read(&hi2c1,
				   AP33772S_ADDR,
				   0x21 + PDO - 1,             // register address for SRCPDO
				   I2C_MEMADD_SIZE_8BIT,
				   pdo_raw,
				   2,
				   HAL_MAX_DELAY);
	pdo_vals pdo = _extract_PDO(pdo_raw);

	uint8_t voltage = 0;
	if(pdo._pps){
		printf("\r\n\r\nPPS detected. Max voltage is %dV. Select PPS voltage in 100mV units for SPR and 200mV units for EPR:\r\n", PDO <= 7 ? pdo._voltage /10 : pdo._voltage /5);

		voltage = get_user_input();
		buff[0] = voltage;
	} else{
		voltage = pdo._voltage;
		buff[0] = 0xff;
	}

	if(!pdo._detected){
	  printf("PDO %d not detected. Try again.\r\n", PDO);
	} else {
	  printf("\r\nSelect PDO %d: \tVoltage: %dV\tCurrent limit: ", PDO, PDO <= 7 ? voltage /10 : voltage /5);
	  _print_curr_lim(current);
	  printf("\tAbsolute max current: ");
	  _print_max_curr(pdo._current);
	  printf("\tPPS: %s\r\n", (pdo._pps) ? "Yes" : "No");
	}
	printf("Check compatability between max current and set current limit. \r\n\r\nConfirm? (y/n)\r\n");
	uint8_t buf = '\0';
	while(buf == '\0'){
		HAL_UART_Receive(&huart1, &buf, 1, HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart1, &buf, 1, HAL_MAX_DELAY);

	if(buf == 'y'){
		printf("\n\rRequesting PDO...\r\n");
		HAL_I2C_Mem_Write(&hi2c1, AP33772S_ADDR, 0x31, I2C_MEMADD_SIZE_8BIT, buff, 2, HAL_MAX_DELAY);
	}else{
		printf("Aborted.\n\r");
	}
}

void print_stats(){
	printf("Voltage: %hu mV\tCurrent: %hu mA\tTemperature: %hu C\t%d mW\r\n", _get_voltage(), _get_current(), _get_temperature(), _get_voltage()*_get_current()/1000);
}

//void _print_arr(uint8_t *pdo_raw, int bytes){
////	printf("\r\n");
//	int mask = 1 << (8*bytes - 1);
//	for(int i = 0; i <( 8*bytes); i++){
//	  printf("%d", (*pdo_raw & mask)>>((8*bytes)-1-i));
//	  mask >>= 1;
//	}
//	printf("\r\n");
//}

void _print_arr(uint8_t *pdo_raw, int bytes){
    uint32_t value = 0;
    for(int i = bytes-1; i >= 0; i--){
        value = (value << 8) | pdo_raw[i]; // LSB first in value
    }
    printf("\t0x%x\t%d-%d\t",value, pdo_raw[1], pdo_raw[0]);

    for(int i = 8*bytes - 1; i >= 0; i--){
    	if((i+1)%8 == 0){
    		printf(" ");
    	}
        printf("%d", (value >> i) & 1); // LSB first in output
    }
    printf("\r\n");
}
//
//void _print_arr(uint8_t *pdo_raw, int bytes){
//
//	uint32_t value = 0;
//	for(int i = bytes-1; i >= 0; i--){
//		value = (value << 8) | pdo_raw[i]; // LSB first in value
//	}
//
//    for(int i = 0; i < 8; i++){
//    	printf("%d", (pdo_raw[1] & (1 << (7-i))) >> (7-i) );
//    }
//    for(int i = 0; i < 8; i++){
//		printf("%d", pdo_raw[0] & (1 << (7-i)) >> (7-i));
//	}
//    printf("\r\n");
//}




void _get_data(uint8_t addr, int len){
	uint8_t buff[26];  // buffer for all PDOs

	HAL_I2C_Mem_Read(&hi2c1,
				   AP33772S_ADDR,
				   addr,             // register address for SRCPDO
				   I2C_MEMADD_SIZE_8BIT,
				   buff,
				   len,
				   HAL_MAX_DELAY);


	printf("Data @ 0x%x:\r\n\t\t\t", addr);
	_print_arr(buff, len);

}

int _get_voltage(){
	uint8_t buff[2];  // buffer for all PDOs

	HAL_I2C_Mem_Read(&hi2c1,
				   AP33772S_ADDR,
				   0x11,             // register address for SRCPDO
				   I2C_MEMADD_SIZE_8BIT,
				   buff,
				   2,
				   HAL_MAX_DELAY);


	return ((buff[1] << 8) | buff[0]) * 80;
}
int _get_current(){
	uint8_t buff;  // buffer for all PDOs

	HAL_I2C_Mem_Read(&hi2c1,
				   AP33772S_ADDR,
				   0x12,             // register address for SRCPDO
				   I2C_MEMADD_SIZE_8BIT,
				   &buff,
				   1,
				   HAL_MAX_DELAY);


	return buff * 24;
}

int _get_temperature(){
	uint8_t buff;  // buffer for all PDOs

	HAL_I2C_Mem_Read(&hi2c1,
				   AP33772S_ADDR,
				   0x13,             // register address for SRCPDO
				   I2C_MEMADD_SIZE_8BIT,
				   &buff,
				   1,
				   HAL_MAX_DELAY);

	return buff;
}


void set_default_voltage(){
	req_PDO_options();
	for(int i = 0; i < def_pdos; i++){
		printf("Pdo number %d. val %d\r\n", i, def_pdo[i]);
		for(int j = 0; j < pdo_idx; j++){
			printf("\t Comparing Pdo voltage %d at PDO %d. val %d\r\n", j,  pdos[j], pdo_voltages[j]);
			if(pdo_voltages[j] == def_pdo[i]){
				printf("Found!\r\n");
				uint8_t buff[2];
				buff[1] = (pdos[j] << 4) | 0xf; //set max current
				buff[0] = 0xff; //set max voltag
				_print_arr(&buff, 2);
				HAL_I2C_Mem_Write(&hi2c1, AP33772S_ADDR, 0x31, I2C_MEMADD_SIZE_8BIT, buff, 2, HAL_MAX_DELAY);
				return;
			}
		}
	}
}
