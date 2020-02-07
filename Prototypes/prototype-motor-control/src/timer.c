/*
 * timer.c
 *
 *  Created on: Feb 4, 2020
 *      Author: gufu
 */

#include "inc/timer.h"

volatile bool delay_done = false;

__interrupt void cpu_timer0_isr(void);

void timer_init(void)
{
    InitCpuTimers();

    EALLOW;
    PieVectTable.TINT0 = &cpu_timer0_isr;
    EDIS;

    ConfigCpuTimer(&CpuTimer0, 90, 1000);
    CpuTimer0.RegsAddr->TCR.bit.TIE = 0;
    IER |= M_INT1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}

void delay_1ms(void)
{
    // Start/Restart timer (TSS bit)
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 1;
    while (!delay_done);
    delay_done = false;
}

__interrupt void cpu_timer0_isr(void)
{
    delay_done = true;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 0;
    CpuTimer0Regs.TCR.bit.TIF = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
