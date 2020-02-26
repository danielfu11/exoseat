/*
 * timer.h
 *
 *  Created on: Feb 4, 2020
 *      Author: gufu
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "DSP28x_Project.h"
#include "inc/commutation.h"
#include "inc/speed_control.h"
#include <stdbool.h>

void timer_init(void);

void delay_1ms(void);

#endif /* INC_TIMER_H_ */
