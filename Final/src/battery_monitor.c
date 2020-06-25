/*
 * battery_monitor.c
 *
 *  Created on: Mar 14, 2020
 *      Author: gufu
 */

#include "inc/battery_monitor.h"
#include "DSP28x_Project.h"

extern volatile Uint16 PVDD;

__interrupt void adc_isr(void);

void battery_monitor_init(void)
{
    EALLOW;
    PieVectTable.ADCINT1 = &adc_isr;
    EDIS;

    InitAdc();
    AdcOffsetSelfCal();

    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
    IER |= M_INT1;

    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1; // Enable non-overlap mode

    //
    // ADCINT1 trips after AdcResults latch
    //
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    AdcRegs.INTSEL1N2.bit.INT1E     = 1;  // Enabled ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;  // Disable ADCINT1 Continuous mode

    //
    // setup EOC1 to trigger ADCINT1 to fire
    //
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 1;

    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 0xA;  // set SOC0 channel select to ADCINB2
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 2;

    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 1; // set SOC0 start trigger on CpuTimer0
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 1; // set SOC1 start trigger on CpuTimer0

    //
    // set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
    //
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 6;

    //
    // set SOC1 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
    //
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 6;
    EDIS;
}

__interrupt void adc_isr(void)
{
    PVDD = AdcResult.ADCRESULT0;

    //
    // Clear ADCINT1 flag reinitialize for next SOC
    //
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE
}

