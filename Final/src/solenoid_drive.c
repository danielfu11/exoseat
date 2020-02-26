/*
 * solenoid_drive.c
 *
 *  Created on: Feb 3, 2020
 *      Author: gufu
 */

#include "inc/solenoid_drive.h"


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

    CpuTimer0.InterruptCount = 0;
    // Start/Restart timer (TSS bit)
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 1;
    while (CpuTimer0.InterruptCount < 45);

    GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;
}

void pawl_down(void)
{
    GpioDataRegs.GPASET.bit.GPIO23 = 1;

    CpuTimer0.InterruptCount = 0;
    // Start/Restart timer (TSS bit)
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 1;
    while (CpuTimer0.InterruptCount < 40);

    GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;
}
