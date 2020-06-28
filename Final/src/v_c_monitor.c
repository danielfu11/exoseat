/*
 * battery_monitor.c
 *
 *  Created on: Mar 14, 2020
 *      Author: gufu
 */

#include <stdbool.h>
#include <inc/v_c_monitor.h>
#include "DSP28x_Project.h"

#define PVDD_THRESHOLD 2220 //2220 = 24v (less than 24V means 10% or less charge)
#define PVDD_DISCARDVAL 250 //voltage when only the USB is plugged in

static volatile Uint16 PVDD;
static volatile Uint16 ISENA;
static volatile Uint16 ISENB;
static volatile Uint16 ISENC;

#ifdef FLASH_MODE
#pragma CODE_SECTION(adc_isr, "ramfuncs");
#endif

__interrupt void adc_isr(void);

void v_c_monitor_init(void)
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
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 0x0;  // set SOC0 channel select to ADCINA0
    AdcRegs.ADCSOC2CTL.bit.CHSEL    = 0x8;  // set SOC0 channel select to ADCINB0
    AdcRegs.ADCSOC3CTL.bit.CHSEL    = 0x1;  // set SOC0 channel select to ADCINA1

    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 1; // set SOC0 start trigger on CpuTimer0
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 1; // set SOC1 start trigger on CpuTimer0
    AdcRegs.ADCSOC2CTL.bit.TRIGSEL  = 1; // set SOC2 start trigger on CpuTimer0
    AdcRegs.ADCSOC3CTL.bit.TRIGSEL  = 1; // set SOC3 start trigger on CpuTimer0

    //
    // set SOC0, SOC1, SOC2, SOC3  S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
    //
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 6;
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 6;
    AdcRegs.ADCSOC2CTL.bit.ACQPS    = 6;
    AdcRegs.ADCSOC3CTL.bit.ACQPS    = 6;

    EDIS;
}


bool current_monitor(void)
{
    bool status = false;

}

bool battery_low(void)
{
    bool status = false;
    if((PVDD > PVDD_DISCARDVAL) && (PVDD < PVDD_THRESHOLD))
    {
        status = true;// battery is at 10%
    }
    return status;
}

__interrupt void adc_isr(void)
{
    PVDD = AdcResult.ADCRESULT0;
    ISENA = AdcResult.ADCRESULT1;
    ISENB = AdcResult.ADCRESULT2;
    ISENC = AdcResult.ADCRESULT3;

    //
    // Clear ADCINT1 flag reinitialize for next SOC
    //
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE
}

