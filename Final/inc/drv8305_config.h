/*
 * drv8305_config.h
 *
 *  Created on: Feb 1, 2020
 *      Author: gufu
 */

#ifndef INC_DRV8305_CONFIG_H_
#define INC_DRV8305_CONFIG_H_

#include <stdbool.h>

void drv8305_init(void);

void enable_drv8305(void);

bool fault_cleared(void);

#endif /* INC_DRV8305_CONFIG_H_ */
