/*
 * state_machine.c
 *
 *  Created on: Feb 28, 2020
 *      Author: sonya
 */

state_e state;
extern Uint8 hall_state;

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

void state_machine(commands_e command)
{
    switch(state)
    {
        case IDLE:
            if(command == PULL_ME_UP)
            {
                trasnition_to_moving_up();
            }
            break;

        case MOVING_UP:
            if(command == STOP)
            {
                transition_to_locked_midway();
            }
            if((command == ENC) || (command == PSENSE))
            {
                transition_to_locked_upright();
            }
            if (new_hall_state)
            {
                feedback = calculate_speed();
                hall_state = read_hall_states();
                drive_state = next_commutation_state(CW, hall_state, false);
                new_hall_state = false;
            }
            break;

        case LOCKED_MIDWAY:
            if(command == BRING_ME_DOWN)
            {
                transition_to_idle();
            }
            break;

        case LOCKED_UPRIGHT:
            if(command == BRING_ME_DOWN)
            {
                transition_to_idle();
            }
            break;

        default:
            // Error
            break;
    }
}

void transition_to_idle(void)
{
    motor_off();
    pawl_release();
}

void transition_to_locked_upright(void)
{
    pawl_down();
    motor_off();
}

void transition_to_locked_midway(void)
{
    pawl_down();
    motor_off();
}

void trasnition_to_moving_up(void)
{
    pawl_down();
    motor_on();
}
