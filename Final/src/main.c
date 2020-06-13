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
//#include "inc/spi.h"
#include "inc/debug.h"
#include "inc/state_machine.h"
#include "inc/command_queue.h"
#include "inc/user_control.h"
#include "inc/prox_sense.h"

extern volatile bool new_hall_state;
extern volatile bool is_hall_prox_on_latch;

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

drv8305_fault_regs_t fault_regs =
{
 .ic_faults = 0,
 .ov_vds_faults = 0,
 .vgs_faults = 0,
 .warnings = 0,
};

direction_e direction;

//#define DISABLE_STATE_MACHINE
//#define FLASH_MODE

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




#ifdef FLASH_MODE
    //
    // Copy time critical code and Flash setup code to RAM
    // This includes the following ISR functions: epwm1_timer_isr(),
    // epwm2_timer_isr(), epwm3_timer_isr and and InitFlash();
    // The  RamfuncsLoadStart, RamfuncsLoadSize, and RamfuncsRunStart
    // symbols are created by the linker. Refer to the F2808.cmd file.
    //
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32)&RamfuncsLoadSize);
#endif
    //
    // Call Flash Initialization to setup flash waitstates
    // This function must reside in RAM
    //
    InitFlash();
#endif






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

    // Initialize hall sensor GPIO pins
    hall_prox_sense_init();

    // Initialize SPI peripheral
    spi_init();

    // Enable global interrupts
    EINT;

    // Enable DRV8305 if nFault
    enable_drv8305();
    delay_1ms();
    // TODO: Disable drv8305 during IDLE state?

    direction = CCW; //CCW = wrap up

    if(!write_drv8305_reg(SPI_REG_ADDR_IC_OPERATION, 0x0620)) //disable PVDD_UVLO2 fault, enable OTSD (overtemp shutdown)
    {
        while(1);
    }

#ifdef DISABLE_STATE_MACHINE
    // Read initial hall sensor states
    Uint8 hall_state = read_hall_states();

    // Commutate=-
    phase_drive_s drive_state = next_commutation_state(direction, hall_state, true);
#endif

    int hall_cntr = 0; //for debugging

    commands_e received_command = NO_COMMAND;

#ifdef DISABLE_STATE_MACHINE
    int i;
#endif

    while (1)
    {

        if(!read_drv8305_fault_regs(&fault_regs))
        {
            //while(1); //stop to check fault (only keep for debugging)
        }

        if(is_hall_prox_on_latch)
        {
            hall_cntr++;
           // is_hall_prox_on_latch = false;
        }

        if (fault_cleared())
        {
#ifndef DISABLE_STATE_MACHINE
            // If new command
            if (!queue_pop(&command_q, &received_command))
            {
                //__asm("     ESTOP0");
            }
            state_machine(received_command);
#else
            if (new_hall_state)
            {
                feedback = (float) calculate_speed();
                //speed_arr[i] = feedback;
                hall_state = read_hall_states();
                drive_state = next_commutation_state(direction, hall_state, false);
                new_hall_state = false;
                i++;
                if (i == 12) i = 0;
                //if (i == 2) blah = 1;
            }
#endif
        }
        else
        {
//            enable_drv8305();
//            delay_1ms();
        }
    }
}
