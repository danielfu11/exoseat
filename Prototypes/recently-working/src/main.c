/*
 * main.c
 *
 *  Created on: Feb 1, 2020
 *      Author: gufu
 */

#include "DSP28x_Project.h"

#include "inc/hallsensor.h"
#include "inc/commutation.h"
#include "inc/drv8305_config.h"
#include "inc/timer.h"
#include "inc/speed_control.h"
#include "inc/spi.h"
#include "inc/debug.h"
#include "inc/state_machine.h"
#include "inc/command_queue.h"
#include "inc/user_control.h"

extern volatile bool new_hall_state;

DCL_PID pid_controller = PID_DEFAULTS;  //initialize before interrupts turn on
float reference;                        // controller set-point reference (rk)
float feedback;                         // measured feedback value (yk)
float saturation;                       // external output clamp flag (lk)
float control_output;                   // output of controller block (uk)

command_queue_t command_q =
{
 .head = 0,
 .tail = 0,
 .size = 0,
};

//
//extern volatile Uint16 rdata_spi;
//extern volatile Uint8 spi_done;
//
//Uint16 ov_vds_fault, ic_fault, vgs_fault, test_reg, warn_watchdog;

void main(void)
{
    // Disable watchdog timer, enable clocks
    InitSysCtrl();

    // Disable all interrupts at the CPU level
    DINT;

    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    InitPieVectTable();

    // Initialize timer
    timer_init();

    // Initialize hall sensors
    hallsensor_init();

    // Initialize gate drive pins
    gate_drive_init();

    // Initialize DRV8305
    drv8305_init();

    //Initialize PID speed controller
    controller_init();

    // Initialize UART
    usr_ctrl_comm_init();

    // Enable global interrupts
    EINT;

    // Enable DRV8305 if nFault
    enable_drv8305();
    delay_1ms();
    // TODO: Disable drv8305 during IDLE state?

    // Initialize SPI peripheral
    spi_init();

    //Debug comm init
    //debug_comm_init();

//    // Read initial hall sensor states
//    Uint8 hall_state = read_hall_states();
//
//    // Commutate
//    phase_drive_s drive_state = next_commutation_state(CCW, hall_state, true);


   // send_spi_control_word(SPI_READ, 0x5, 0);


    int i = 0;

    commands_e received_command = NO_COMMAND;

//    queue_push(&command_q, PULL_ME_UP);
    usr_ctrl_send_msg(0x41);

    while (1)
    {
        if (fault_cleared())
        {
            // If new command
            if (!queue_pop(&command_q, &received_command))
            {
                //__asm("     ESTOP0");
            }
            state_machine(received_command);
//            if (new_hall_state)
//            {
//                feedback = (float) calculate_speed();
//                //speed_arr[i] = feedback;
//                hall_state = read_hall_states();
//                drive_state = next_commutation_state(CCW, hall_state, false);
//                new_hall_state = false;
//                i++;
//                if (i == 12) i = 0;
//                //if (i == 2) blah = 1;
//            }
        }
        else
        {
            enable_drv8305();
            delay_1ms();
        }
    }
}
