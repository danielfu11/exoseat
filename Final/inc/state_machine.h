/*
 * state_machine.h
 *
 *  Created on: Feb 28, 2020
 *      Author: sonya
 */

#include "inc/commutation.h"

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

//#define DISABLE_STARTUP_CALIBRATION

typedef enum
{
    NO_COMMAND = 0x00,
    STOP = 0x53,
    BRING_ME_DOWN = 0x42,
    PULL_ME_UP = 0x50,
//    PSENSE,
//    ENC_UP,
//    ENC_DOWN,
} commands_e;

typedef enum
{
    STARTUP,
    STARTUP_DOWN,
    STARTUP_UP,
    IDLE,
    MOVING_UP,
    LOCKED_MIDWAY,
    LOCKED_UPRIGHT,
    MOVING_DOWN,
} state_e;

typedef enum
{
    DIRECTION_UP = CW,
    DIRECTION_DOWN = CCW
} position_direction_e;

typedef struct
{
    // full distance between IDLE state and LOCKED_UPRIGHT state
    // (this var is modified when prox sensor is activated)
    Uint32 full_distance;

    // distance left between current position and final position
    Uint32 desired_distance;

    // distance moved from IDLE or LOCKED_UPRIGHT
    volatile Uint32 distance_moved;

    // if in MOVING_UP or MOVING_DOWN state, direction == DIRECTION_UP or DIRECTION_DOWN, respectively
    // if in LOCKED_MIDWAY, LOCKED_UPRIGHT, or IDLE state, direction == the last direction the system moved in
    //      (i.e. if prev state was MOVING_UP, direction == DURECTION_UP, etc)
    position_direction_e direction;
} position_tracker_t;

void state_machine(commands_e command);

#endif /* INC_STATE_MACHINE_H_ */
