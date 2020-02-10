/*
 * hallsensor.h
 *
 *  Created on: Feb 1, 2020
 *      Author: gufu
 */

#ifndef INC_HALLSENSOR_H_
#define INC_HALLSENSOR_H_

#include "DSP28x_Project.h"
#include <stdbool.h>
#include "inc/timer.h"

void hallsensor_init(void);

Uint8 read_hall_states(void);

Uint32 calculate_speed(void);

#endif /* INC_HALLSENSOR_H_ */
