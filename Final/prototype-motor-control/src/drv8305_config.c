/*
 * drv8305_config.c
 *
 *  Created on: Feb 1, 2020
 *      Author: gufu
 */

#include "inc/drv8305_config.h"

void drv8305_init(void)
{
    EALLOW;

    // EN_GATE
    GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0;
    GpioDataRegs.GPBCLEAR.bit.GPIO50 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;

    // nFAULT
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO28 = 0;
    EDIS;
}

void enable_drv8305(void)
{
    GpioDataRegs.GPBSET.bit.GPIO50 = 1;
}

bool fault_cleared(void)
{
    if (0 == GpioDataRegs.GPADAT.bit.GPIO28)
    {
        return false;
    }
    else if (1 == GpioDataRegs.GPADAT.bit.GPIO28)
    {
        return true;
    }
}
