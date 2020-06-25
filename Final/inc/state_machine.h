/*
 * state_machine.h
 *
 *  Created on: Feb 28, 2020
 *      Author: sonya
 */

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

typedef enum
{
    NO_COMMAND = 0x00,
    STOP = 0x53,
    BRING_ME_DOWN = 0x42,
    PULL_ME_UP = 0x50,
    PSENSE,
    ENC_UP,
    ENC_DOWN,
} commands_e;

typedef enum
{
    IDLE,
    MOVING_UP,
    LOCKED_MIDWAY,
    LOCKED_UPRIGHT,
    MOVING_DOWN,
} state_e;

void state_machine(commands_e command);

#endif /* INC_STATE_MACHINE_H_ */
