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

void controller_init(void);

void update_reference(float32_t new_ref);

#endif /* SPEED_CONTROL_H_ */
