/*
 * prox_sense.c
 *
 *  Created on: Mar 11, 2020
 *      Author: sonya
 */

#include "DSP28x_Project.h"
#include "inc/prox_sense.h"


void hall_prox_sense_init(void)
{
    //
    // GPIO27 and GPIO26 are inputs
    //
    EALLOW;
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;        // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 0;         // input
    GpioCtrlRegs.GPAQSEL2.bit.GPIO27 = 0;       // Synch to SYSCLKOUT only

    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;        // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 0;         // input
    GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 0;       // Synch to SYSCLKOUT only


    //TODO: enable internal pullup resistor?
}


bool poll_prox_sense(void)
{
    //check if either hall sensor is activated (NOTE: sensor is active low)
    return ((!GpioDataRegs.GPADAT.bit.GPIO27) | (!GpioDataRegs.GPADAT.bit.GPIO26));
}

