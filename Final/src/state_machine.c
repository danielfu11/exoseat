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
#include "inc/speed_control.h"

#define ONE_REV                   12
#define DISTANCE_REQUIRED         (2*40*ONE_REV)*1.2 // Should be one full revolution on outer shaft
#define LOCKED_UPRIGHT_POSITION   0
#define IDLE_POSITON              LOCKED_UPRIGHT_POSITION + DISTANCE_REQUIRED

#define END_DIST                  40*ONE_REV //Distance from magnets to hardstop (currently 1/4 of total distance but TODO: change to actual distance when it is measured)
#define START_CAL_DIST            END_DIST + DISTANCE_REQUIRED/8 //END_DIST + a little more
#define MAX_STARTUP_UP_DIST       1000000 //2 * START_CAL_DIST //TODO: calculate exact distance for this once it is on the chair and rope is cut
#define CORRECTION_AMOUNT         100 //30

#define STARTUP_SPEED             700.0f
#define NORMAL_OPERATION_SPEED    700.0f
#define CORRECTION_SPEED          300.0f


#define DISABLE_SLIP_CORRECTION


#ifdef DISABLE_STARTUP_CALIBRATION
state_e state = IDLE;
#else
state_e state = STARTUP;
#endif

//Uint32 desired_distance = DISTANCE_REQUIRED;
extern Uint8 hall_state;
extern float feedback;
extern volatile bool new_hall_state;
//extern volatile Uint32 distance_moved;
extern volatile bool is_hall_prox_on_latch;

position_tracker_t position =
{
//   .full_distance = DISTANCE_REQUIRED,
     .desired_distance = DISTANCE_REQUIRED,
     .distance_moved = 0, // 1 tick == 30 deg, distance moved since last state transition
     .distance_from_upright = LOCKED_UPRIGHT_POSITION, // 1 tick == 30 deg
     .direction = DIRECTION_DOWN, //direction initialized to down bc system starts in IDLE state
     .correction_type = CORR_NONE,
};

// Private functions
static void transition_to_idle(void)
{
    motor_off();
//    DELAY_US(1000000); //delay to take into account inertia of motor (takes a little while to fully stop)
//    pawl_release();
    position.distance_moved = 0; //not needed now that dist from upright is used
    state = IDLE;
}

static void transition_to_locked_upright(void)
{
//    pawl_down();
    motor_off();
    DELAY_US(1000000); //delay to take into account inertia of motor (takes a little while to fully stop)
    position.distance_moved = 0;
    position.distance_from_upright = LOCKED_UPRIGHT_POSITION;
    state = LOCKED_UPRIGHT;
}

static void transition_to_locked_midway(void)
{
//    pawl_down();
    motor_off();
    //DELAY_US(1000000); //delay to take into account inertia of motor (takes a little while to fully stop)
    position.distance_moved = 0; //reset distance moved
    state = LOCKED_MIDWAY;
}

static void trasnition_to_moving_up(void)
{
//    pawl_down();

    if (position.correction_type == CORR_NONE)
    {
        update_reference(NORMAL_OPERATION_SPEED);
    }
    else
    {
        update_reference(CORRECTION_SPEED);
        position.desired_distance = position.distance_moved; // move back up the distance that it slipped
        if(position.direction == DIRECTION_UP)
        {
            position.distance_from_upright += (2 * position.desired_distance);
        }
    }
    position.direction = DIRECTION_UP;
    position.distance_moved = 0;
    motor_on(CW);
    state = MOVING_UP;
}

static void transition_to_moving_down(void)
{
//    pawl_release();
    position.direction = DIRECTION_DOWN;
    position.distance_moved = 0;
    update_reference(NORMAL_OPERATION_SPEED);
    motor_on(CCW);
    state = MOVING_DOWN;
}

inline static void startup_move_down(void){
//    pawl_release();
    position.direction = DIRECTION_DOWN;
    position.distance_moved = 0;
    motor_on(CCW);
    state = STARTUP_DOWN;
}

inline static void startup_move_up(void)
{
//    pawl_down();
    position.direction = DIRECTION_UP;
    position.distance_moved = 0;
    motor_on(CW);
    state = STARTUP_UP;
}

inline static void transition_to_error_state(void)
{
    motor_off(); // if use turns motor off during calibration, assume something went wrong, and just turn off motor - user will have to turn power on and off to restart calibration
    state = ERROR;
}

// Public functions
void state_machine(commands_e command)
{
    phase_drive_s drive_state;
    switch(state)
    {
        case STARTUP:
            update_reference(STARTUP_SPEED);
            startup_move_down();
            break;

        case STARTUP_DOWN:
            if(command == STOP)
            {
                transition_to_error_state(); //assume a stop during calibration means something went wrong
            }
            else if(position.distance_moved >= START_CAL_DIST)
            {
                motor_off();
                DELAY_US(1000000);
                startup_move_up();
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

        case STARTUP_UP:
            if(command == STOP)
            {
                transition_to_error_state(); //assume a stop during calibration means something went wrong
            }
            else if (is_hall_prox_on_latch == true)
            {
                transition_to_locked_upright();
                is_hall_prox_on_latch = false;
            }
            else if (position.distance_moved >= MAX_STARTUP_UP_DIST) //last resort safety check if prox sensor is not working (so that motor doesn't keep wrapping up forever)
            {
                transition_to_error_state(); //prox sensor was not found
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

        case IDLE:
            if(command == PULL_ME_UP)
            {
                position.desired_distance = position.distance_from_upright;
                trasnition_to_moving_up();
            }
#ifdef DISABLE_SLIP_CORRECTION
            else if(position.distance_moved >= CORRECTION_AMOUNT) // if the motor is slipping without being turned on
            {
                position.correction_type = CORR_IDLE;
                trasnition_to_moving_up(); // (if motor is moving while in a stopped state, we ASSUME the rope is rolling out (i.e. the use is moving down)
            }
#endif
            break;

        case MOVING_UP:
            if(command == STOP)
            {
                transition_to_locked_midway();
            }
            else if ((position.distance_moved >= position.desired_distance) || (is_hall_prox_on_latch == true))
            {
#ifdef DISABLE_SLIP_CORRECTION
                if(position.correction_type == CORR_NONE || (is_hall_prox_on_latch == true))
                {
                    transition_to_locked_upright(); // distance moved is set to 0 in here
                }
                else if (position.correction_type == CORR_LOCKED_UPRIGHT)
                {
                    transition_to_locked_upright();
                }
                else if (position.correction_type == CORR_IDLE)
                {
                    transition_to_idle();
                }
                else if (position.correction_type == CORR_LOCKED_MIDWAY)
                {
                    transition_to_locked_midway();
                }
                position.correction_type = CORR_NONE;
#else
                transition_to_locked_upright(); // distance moved is set to 0 in here
#endif
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
                position.desired_distance = DISTANCE_REQUIRED - position.distance_from_upright;
                position.distance_moved = 0; //reset distance moved
                transition_to_moving_down();
            }
            else if (command == PULL_ME_UP)
            {
                position.desired_distance = position.distance_from_upright;
                position.distance_moved = 0; //reset distance moved
                trasnition_to_moving_up();
            }
#ifdef DISABLE_SLIP_CORRECTION
            else if(position.distance_moved >= CORRECTION_AMOUNT) // if the motor is slipping without being turned on
            {
                position.correction_type = CORR_LOCKED_MIDWAY;
                trasnition_to_moving_up(); // (if motor is moving while in a stopped state, we ASSUME the rope is rolling out (i.e. the use is moving down)
            }
#endif
            break;

        case LOCKED_UPRIGHT:
            if(command == BRING_ME_DOWN)
            {
                position.desired_distance = DISTANCE_REQUIRED;
                transition_to_moving_down();
            }
#ifdef DISABLE_SLIP_CORRECTION
            // software lock (aka proportional position control system):
            else if(position.distance_moved >= CORRECTION_AMOUNT) // if the motor is slipping a certain amount without being turned on (not responsive enough for "if distance moved is greater than 0." More accurate recovery if larger number
            {
                position.correction_type = CORR_LOCKED_UPRIGHT;
                trasnition_to_moving_up(); // (if motor is moving while in a stopped state, we ASSUME the rope is rolling out (i.e. the use is moving down)
            }
#endif
            break;

        case MOVING_DOWN:
            update_reference(700.0f);
            if(command == STOP)
            {
                transition_to_locked_midway();
            }
            else if (position.distance_moved >= position.desired_distance)
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

        case ERROR:
            while(1);
            break;

        default:
            // Error
            break;
    }
}
