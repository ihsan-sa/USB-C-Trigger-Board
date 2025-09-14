/*
 * AP33772S.h
 *
 *  Created on: Aug 29, 2025
 *      Author: ihsan
 */

#ifndef SRC_AP33772S_H_
#define SRC_AP33772S_H_

static const uint8_t AP33772S_ADDR = 0x52 << 1; //leave one bit for R/W
static const uint8_t def_pdo[] = {20};
static const uint8_t def_pdos = 1;

void print_stats();
void req_PDO_options();
void select_PDO(uint8_t PDO, uint8_t current);
void set_default_voltage();



#endif /* SRC_AP33772S_H_ */
