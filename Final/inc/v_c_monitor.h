/*
 * battery_monitor.h
 *
 *  Created on: Mar 14, 2020
 *      Author: gufu
 */

#ifndef INC_V_C_MONITOR_H_
#define INC_V_C_MONITOR_H_

#define OVERCURRENT_VAL_AMPS        3 // Amps
#define CURR_SENSE_RESISTOR_VAL     0.007f
#define OVERCURRENT_VAL_VOLTS       (OVERCURRENT_VAL_AMPS*CURR_SENSE_RESISTOR_VAL)

void v_c_monitor_init(void);
bool is_overcurrent(void);
bool battery_low(void);

#endif /* INC_V_C_MONITOR_H_ */
