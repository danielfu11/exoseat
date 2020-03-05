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
    NO_COMMAND,
    STOP,
    BRING_ME_DOWN,
    PULL_ME_UP,
    PSENSE,
    ENC,
} commands_e;

typedef enum
{
    IDLE,
    MOVING_UP,
    LOCKED_MIDWAY,
    LOCKED_UPRIGHT,
} state_e;

void state_machine(void);

#endif /* INC_STATE_MACHINE_H_ */
