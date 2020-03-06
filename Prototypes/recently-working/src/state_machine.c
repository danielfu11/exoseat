/*
 * state_machine.c
 *
 *  Created on: Feb 28, 2020
 *      Author: sonya
 */

#include "DSP28x_Project.h"
#include "inc/state_machine.h"
#include "inc/command_queue.h"
#include "inc/commutation.h"
#include "inc/solenoid_drive.h"
#include "inc/hallsensor.h"

#define ONE_REV                   12
#define DISTANCE_REQUIRED         40*ONE_REV // Should be one full revolution on outer shaft

state_e state = IDLE;
Uint32 desired_distance = DISTANCE_REQUIRED;
extern Uint8 hall_state;
extern float feedback;
extern volatile bool new_hall_state;
extern volatile Uint32 distance_moved;

// Private functions
static void transition_to_idle(void)
{
    motor_off();
    pawl_release();
    distance_moved = 0;
    state = IDLE;
}

static void transition_to_locked_upright(void)
{
    pawl_down();
    motor_off();
    distance_moved = 0;
    state = LOCKED_UPRIGHT;
}

static void transition_to_locked_midway(void)
{
    pawl_down();
    motor_off();
    state = LOCKED_MIDWAY;
}

static void trasnition_to_moving_up(void)
{
    pawl_down();
    motor_on(CW);
    state = MOVING_UP;
}

static void transition_to_moving_down(void)
{
    pawl_release();
    motor_on(CCW);
    state = MOVING_DOWN;
}

// Public functions
void state_machine(commands_e command)
{
    phase_drive_s drive_state;
    switch(state)
    {
        case IDLE:
            if(command == PULL_ME_UP)
            {
                desired_distance = DISTANCE_REQUIRED;
                trasnition_to_moving_up();
            }
            break;

        case MOVING_UP:
            if(command == STOP)
            {
                transition_to_locked_midway();
            }
            else if(distance_moved == desired_distance || (command == PSENSE))
            {
                transition_to_locked_upright();
            }
            else
            {
                if (new_hall_state)
                {
                    feedback = calculate_speed();
                    hall_state = read_hall_states();
                    drive_state = next_commutation_state(CW, hall_state, false);
                    new_hall_state = false;
                }
            }
            break;

        case LOCKED_MIDWAY:
            if(command == BRING_ME_DOWN)
            {
                desired_distance = distance_moved;
                distance_moved = 0;
                transition_to_moving_down();
            }
            else if (command == PULL_ME_UP)
            {
                // Desired distance does not change since she was already going up
                trasnition_to_moving_up();
            }
            break;

        case LOCKED_UPRIGHT:
            if(command == BRING_ME_DOWN)
            {
                transition_to_moving_down();
            }
            break;

        case MOVING_DOWN:
            if (distance_moved == desired_distance)
            {
                transition_to_idle();
            }
            else
            {
                if (new_hall_state)
                {
                    feedback = calculate_speed();
                    hall_state = read_hall_states();
                    drive_state = next_commutation_state(CCW, hall_state, false);
                    new_hall_state = false;
                }
            }
            break;

        default:
            // Error
            break;
    }
}
