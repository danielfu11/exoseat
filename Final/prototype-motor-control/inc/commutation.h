/*
 * commutation.h
 *
 *  Created on: Feb 2, 2020
 *      Author: gufu
 */

#ifndef INC_COMMUTATION_H_
#define INC_COMMUTATION_H_

#include "DSP28x_Project.h"
#include <stdbool.h>

typedef enum
{
    CW,
    CCW
} direction_e;

typedef enum
{
    NC,  // Not connected
    NEG, // Low side drive
    POS  // High side drive
} phase_state;

typedef struct
{
    phase_state phase_u;
    phase_state phase_v;
    phase_state phase_w;
} phase_drive_s;

void gate_drive_init(void);

phase_drive_s next_commutation_state(direction_e dir, Uint8 hall_state, bool startup);

#endif /* INC_COMMUTATION_H_ */
