/*
 * timer.c
 *
 *  Created on: Feb 4, 2020
 *      Author: gufu
 */

#include "inc/timer.h"

extern volatile Uint32 duty_cycle;
extern DCL_PID pid_controller;                  //initialize before interrupts turn on
extern float reference;                        // controller set-point reference (rk)
extern float feedback;                         // measured feedback value (yk)
extern float saturation;                       // external output clamp flag (lk)
extern float control_output;                   // output of controller block (uk)
extern volatile Uint8 blah;

volatile bool delay_done = false;
volatile Uint8 wrap_around = 0;

__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);

#define CPU_FREQ        90U
#define TIMER0_PERIOD   1000U // in microseconds
#define TIMER1_PERIOD   100U
#define TIMER2_PERIOD   100U

void timer_init(void)
{
    InitCpuTimers();

    EALLOW;
    PieVectTable.TINT0 = &cpu_timer0_isr;
    PieVectTable.TINT1 = &cpu_timer1_isr;
    PieVectTable.TINT2 = &cpu_timer2_isr;
    EDIS;

    // CpuTimer0 & CpuTimer1
    CpuTimer0.CPUFreqInMHz = CPU_FREQ;
    CpuTimer0.PeriodInUSec = TIMER0_PERIOD;
    CpuTimer0.RegsAddr->PRD.all = (Uint32) (CPU_FREQ * TIMER0_PERIOD) - 1;
    CpuTimer1.CPUFreqInMHz = CPU_FREQ;
    CpuTimer1.PeriodInUSec = TIMER1_PERIOD;
    CpuTimer1.RegsAddr->PRD.all = (Uint32) (CPU_FREQ * TIMER1_PERIOD) - 1;
    CpuTimer2.CPUFreqInMHz = CPU_FREQ;
    CpuTimer2.PeriodInUSec = TIMER1_PERIOD;
    CpuTimer2.RegsAddr->PRD.all = (Uint32) (CPU_FREQ * TIMER2_PERIOD) - 1;

    // Set pre-scale counter to divide by 1 (SYSCLKOUT)
    CpuTimer0.RegsAddr->TPR.all  = 0;
    CpuTimer0.RegsAddr->TPRH.all  = 0;
    CpuTimer1.RegsAddr->TPR.all  = 0;
    CpuTimer1.RegsAddr->TPRH.all  = 0;
    CpuTimer2.RegsAddr->TPR.all  = 0;
    CpuTimer2.RegsAddr->TPRH.all  = 0;

    //
    // 1 = Stop timer, 0 = Start/Restart Timer
    //
    CpuTimer0.RegsAddr->TCR.bit.TSS = 1;
    CpuTimer1.RegsAddr->TCR.bit.TSS = 1;
    CpuTimer2.RegsAddr->TCR.bit.TSS = 1;

    CpuTimer0.RegsAddr->TCR.bit.TRB = 1;      // 1 = reload timer
    CpuTimer0.RegsAddr->TCR.bit.SOFT = 0;
    CpuTimer0.RegsAddr->TCR.bit.FREE = 0;     // Timer Free Run Disabled
    CpuTimer1.RegsAddr->TCR.bit.TRB = 1;      // 1 = reload timer
    CpuTimer1.RegsAddr->TCR.bit.SOFT = 0;
    CpuTimer1.RegsAddr->TCR.bit.FREE = 0;     // Timer Free Run Disabled
    CpuTimer2.RegsAddr->TCR.bit.TRB = 1;      // 1 = reload timer
    CpuTimer2.RegsAddr->TCR.bit.SOFT = 0;
    CpuTimer2.RegsAddr->TCR.bit.FREE = 0;     // Timer Free Run Disabled

    // 0 = Disable/ 1 = Enable Timer Interrupt
    CpuTimer0.RegsAddr->TCR.bit.TIE = 0;
    CpuTimer1.RegsAddr->TCR.bit.TIE = 1;
    CpuTimer2.RegsAddr->TCR.bit.TIE = 1;

    // Reset interrupt counter
    CpuTimer0.InterruptCount = 0;
    CpuTimer1.InterruptCount = 0;

    // Start timer 1 & 2
    CpuTimer1Regs.TCR.all = 0x4000;
    CpuTimer2Regs.TCR.all = 0x4000;

    IER |= M_INT1 | M_INT13 | M_INT14;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}

void delay_1ms(void)
{
    // Start/Restart timer (TSS bit)
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer0.InterruptCount = 0;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 1;
    while (!delay_done);
    delay_done = false;
}

__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0Regs.TCR.bit.TIF = 1;
    delay_done = true;
    CpuTimer0.InterruptCount++;
    CpuTimer0.RegsAddr->TCR.bit.TIE = 0;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void cpu_timer1_isr(void)
{
    CpuTimer1Regs.TCR.bit.TIF = 1;
    CpuTimer1.InterruptCount++;
    if (CpuTimer1.InterruptCount == 0xFFFF)
    {
        wrap_around++;
        CpuTimer1.InterruptCount = 0;
    }
}

__interrupt void cpu_timer2_isr(void)
{
    CpuTimer2Regs.TCR.bit.TIF = 1;
    if (blah)
    {
        saturation = feedback;
        DCL_runClamp_C1(&saturation, pid_controller.Umax, pid_controller.Umin); //converts saturation to 1 or 0

        control_output = DCL_runPID_C4(&pid_controller, reference, feedback, saturation);

        duty_cycle = (Uint32) (((reference + control_output) / 3300) * 2000 );
        //duty_cycle = (Uint32) ((control_output + 3300.0)/3.3); // 0 <= (control_output - pid_controller.Umin)/3.3) <= 6600/3.3 = 2000 = max duty cycle


        EPwm1Regs.ETSEL.bit.INTEN = 1;
        EPwm2Regs.ETSEL.bit.INTEN = 1;
        EPwm3Regs.ETSEL.bit.INTEN = 1;
    }
}

