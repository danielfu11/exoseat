/*
 * speed_control.h
 *
 *  Created on: Jan 27, 2020
 *      Author: sonya
 */

#ifndef SPEED_CONTROL_H_
#define SPEED_CONTROL_H_

#include "DCL.h"
#include "DCLF32.h" //32 bit floating point implementation

#define INITIAL_REFERENCE_SPEED 75.0f //rpm of motor shaft before gears


void controller_init(void);




#endif /* SPEED_CONTROL_H_ */
