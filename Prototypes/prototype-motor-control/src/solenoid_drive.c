/*
 * solenoid_drive.c
 *
 *  Created on: Feb 3, 2020
 *      Author: gufu
 */

#include "inc/solenoid_drive.h"
#include "inc/timer.h"

extern volatile bool delay_done;

void solenoid_init(void)
{
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;
    GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;
}

void pawl_release(void)
{
    GpioDataRegs.GPASET.bit.GPIO21 = 1;

    CpuTimer0.RegsAddr->PRD.all = (45 * 90 * 1000) - 1;
    // Start/Restart timer (TSS bit)
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 1;
    while (!delay_done);

    GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;
    delay_done = false;
}

void pawl_down(void)
{
    GpioDataRegs.GPASET.bit.GPIO23 = 1;

    CpuTimer0.RegsAddr->PRD.all = (40 * 90 * 1000) - 1;
    // Start/Restart timer (TSS bit)
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 1;
    while (!delay_done);

    GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;
    delay_done = false;
}
