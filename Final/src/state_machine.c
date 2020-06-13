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
#define DISTANCE_REQUIRED         2*40*ONE_REV // Should be one full revolution on outer shaft

state_e state = IDLE;
//Uint32 desired_distance = DISTANCE_REQUIRED;
extern Uint8 hall_state;
extern float feedback;
extern volatile bool new_hall_state;
//extern volatile Uint32 distance_moved;
extern volatile bool is_hall_prox_on_latch;

position_tracker_t position =
{
     .full_distance = DISTANCE_REQUIRED,
     .desired_distance = DISTANCE_REQUIRED,
     .distance_moved = 0, // 1 tick == 30 deg
     .direction = DIRECTION_DOWN, //direction initialized to down bc system starts in IDLE state
};

// Private functions
static void transition_to_idle(void)
{
    motor_off();
//    pawl_release();
    position.distance_moved = 0;
    state = IDLE;
}

static void transition_to_locked_upright(void)
{
//    pawl_down();
    motor_off();
    position.distance_moved = 0;
    state = LOCKED_UPRIGHT;
}

static void transition_to_locked_midway(void)
{
//    pawl_down();
    motor_off();
    state = LOCKED_MIDWAY;
}

static void trasnition_to_moving_up(void)
{
//    pawl_down();
    position.direction = DIRECTION_UP;
    motor_on(CW);
    state = MOVING_UP;
}

static void transition_to_moving_down(void)
{
//    pawl_release();
    position.direction = DIRECTION_DOWN;
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
                position.desired_distance = position.full_distance;
                trasnition_to_moving_up();
            }
            break;

        case MOVING_UP:
            if(command == STOP)
            {
                transition_to_locked_midway();
            }
            else if((position.distance_moved == position.desired_distance) || (is_hall_prox_on_latch == true))
            {
                // if hall prox is activated before desire_dist is reached, reset full and desired distance to distance moved
                // (prevents drifting of moved distance on rope)
                //position.full_distance = position.distance_moved;
                ////position.desired_distance = position.full_distance;

                transition_to_locked_upright(); // distance moved is set to 0 in here
                is_hall_prox_on_latch = false;
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
                if(position.direction == DIRECTION_DOWN)
                {
                    // desired_distance and distance_moved don't change bc already moving down
                }
                else if(position.direction == DIRECTION_UP)
                {
                    position.desired_distance = position.distance_moved;
                    position.distance_moved = 0; // reset distance moved
                }
                transition_to_moving_down();
            }
            else if (command == PULL_ME_UP)
            {
                if(position.direction == DIRECTION_DOWN)
                {
                    position.desired_distance = position.distance_moved;
                    position.distance_moved = 0; // reset distance moved
                }
                else if(position.direction == DIRECTION_UP)
                {
                    // desired_distance and distance_moved don't change bc already moving up
                }
                trasnition_to_moving_up();
            }
            break;

        case LOCKED_UPRIGHT:
            if(command == BRING_ME_DOWN)
            {
                position.desired_distance = position.full_distance;
                transition_to_moving_down();
            }
            break;

        case MOVING_DOWN:
            if(command == STOP)
            {
                //position.desired_distance = position.full_distance; //TODO: is this needed?
                transition_to_locked_midway();
            }
            else if (position.distance_moved == position.desired_distance)
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


void hall_prox_pseudo_isr(void)
{

}
