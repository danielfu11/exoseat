/*
 * solenoid_drive.h
 *
 *  Created on: Feb 3, 2020
 *      Author: gufu
 */

#ifndef INC_SOLENOID_DRIVE_H_
#define INC_SOLENOID_DRIVE_H_

#include "DSP28x_Project.h"
#include "inc/timer.h"

void solenoid_init(void);

void pawl_release(void);

void pawl_down(void);

#endif /* INC_SOLENOID_DRIVE_H_ */
