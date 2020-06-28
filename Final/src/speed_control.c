/*
 * speed_control.c
 *
 *  Created on: Jan 27, 2020
 *      Author: sonya
 *
 *      Uses the Digital Control Library (DCL) to control motor speed
 *
 */

#include "inc/speed_control.h"

extern DCL_PID pid_controller; //initialize before interrupts turn on
extern float reference; // controller set-point reference (rk)
extern float saturation; // external output clamp flag (lk)

void controller_init(void)
{
    update_reference(700.0f); // initial value for control reference -- start with the motor off

    //TODO: add SPS and CSS
    /* initialize controller variables (right now just unity controller) */
    pid_controller.Kp = 1.0f; //proportional gain
    pid_controller.Ki = 0.0f;
    pid_controller.Kd = 0.0f;
    pid_controller.Kr = 1.0f; // set-point weight
    pid_controller.c1 = 0.0f; // derivative path filer ceoff 1
    pid_controller.c2 = 0.0f;
    pid_controller.d2 = 0.0f; // derivative path filter intermed storage 1
    pid_controller.d3 = 0.0f;
    pid_controller.i10 = 0.0f; // integral path intermed storage
    pid_controller.i14 = 1.0f; // saturation intermed storage
//    pid_controller.Umax = 3300.0f; // upper OUTPUT clamp limit
//    pid_controller.Umin = -3052.0f; // lower OUTPUT clamp limit

    saturation = 1.0f; // control loop not saturated
}

void update_reference(float32_t new_ref)
{
    reference = new_ref;
    pid_controller.Umax = 2640.0f - reference;
    pid_controller.Umin = 123.8f - reference;
}



