/*
 * debug.h
 *
 *  Created on: Mar 1, 2020
 *      Author: sonya
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_


void debug_comm_init(void);

void debug_send_msg(char * msg);

void debug_send_float(float data);

void debug_send_int(Uint16 integer);

#endif /* INC_DEBUG_H_ */
