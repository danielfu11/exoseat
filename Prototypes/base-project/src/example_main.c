/*
 * example_main.c
 *
 * Simple project with CpuTimer example
 *
 */

#include "DSP28x_Project.h"
#include "speed_control.h"

// CpuTimer defines
#define CPU_FREQ_MHZ     90 // This depends on what we set in F2806x_Device.h, default is 90
#define TIMER0_PERIOD_US 500000
#define TIMER1_PERIOD_US 1000

__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void epwm1_isr(void);
__interrupt void epwm2_isr(void);
__interrupt void epwm3_isr(void);
__interrupt void adc_isr(void);
static void configure_ePWM(void);
static void configure_ADC(void);


DCL_PID pid_controller = PID_DEFAULTS; //initialize before interrupts turn on
float reference; // controller set-point reference (rk)
float feedback; // measured feedback value (yk)
float saturation; // external output clamp flag (lk)
float control_output; // output of controller block (uk)

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

    // Map ISRs that you write to corresponding entries in the vector table
    // (function pointer table)
    // EALLOW and EDIS are macros that call asm instructions EALLOW and EDIS
    // These instructions protect control registers and the vector table
    // from being written to. Section 1.6.2 of the uC technical reference manual
    // has a complete list of protected registers
    // Keep these EALLOW and EDIS wrapped blocks as small/readable as possible
    EALLOW;
    PieVectTable.TINT0 = &cpu_timer0_isr;
    PieVectTable.TINT1 = &cpu_timer1_isr;

    PieVectTable.EPWM1_INT = &epwm1_isr;
    PieVectTable.EPWM2_INT = &epwm2_isr;
    PieVectTable.EPWM3_INT = &epwm3_isr;

    PieVectTable.ADCINT1 = &adc_isr;
    EDIS;

    // Initialize all used peripherals
    InitCpuTimers();
    InitAdc();
    AdcOffsetSelfCal();
    configure_ADC();
    configure_ePWM();

    // Timer0: 90MHz, 500ms period
    ConfigCpuTimer(&CpuTimer0, CPU_FREQ_MHZ, TIMER0_PERIOD_US);
    // Timer1: 90MHz, 1ms period
    ConfigCpuTimer(&CpuTimer1, CPU_FREQ_MHZ, TIMER1_PERIOD_US);

    // From TI:
    // To ensure precise timing, use write-only instructions to write to the
    // entire register. Therefore, if any of the configuration bits are changed
    // in ConfigCpuTimer and InitCpuTimers (in F2806x_CpuTimers.h), the
    // below settings must also be updated.
    //
    // My explanation: the way that the registers are defined, doing something
    // that seems to write to a single bit is actually reading the whole register and then
    // writing to that single bit which uses more instructions (therefore more CPU cycles)

    // TIE - Bit 12 = 1
    // FREE = 0 SOFT = 0, which means that when debugging the timer will stop right away
    // TSS Bit 4, write 0 to start or restart the CPU to 0
    // Writing 0 to other bits doesn't do anything
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer1Regs.TCR.all = 0x4000;

    // GPIO34 == D9 LED on launchpad, GPIO52 == pin53 configure as output
    // Table 6-73 in datasheet
    EALLOW;

    // It looks like all registers are bit mapped in the HAL so no
    // need for OR/AND NOT operations
    // There are some exceptions described in 2.6.1 of the FW Development
    // Package User's Guide
    // 1. Registers with multiple flag bits in which writing a 1 clears that flag
    // 2. Registers with volatile bits (for these just don't modify them at all)
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;

    GpioCtrlRegs.GPBMUX2.bit.GPIO52 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;

    EDIS;

    /* initialize PID speed controller */
    ControllerInit();


    // Enable CPU interrupt line INT1 which can be muxed from TINT0
    // Table 6-17 in the uC datasheet has complete mappings for each of
    // the 12 CPU interrupt lines
    // 1 is for GPIO,
    // 1 and 13 is used for CPU-Timer0 and 1
    // 3 is for ePWM
    IER |= M_INT1 | M_INT13 | M_INT3;

    //
    // Enable TINT0 in the PIE: Group 1 interrupt 7
    // EPWM INTn in the PIE: Group 3 interrupt 1-3
    // ADC: Group 1 interrupt 1
    //
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx3 = 1;

    //
    // Enable global Interrupts and higher priority real-time debug events
    //
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM (Still not sure what this is)

    while (1)
    {
    }
}

__interrupt void cpu_timer0_isr(void)
{
    // Toggle LED D9
    GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;

    // Clear timer int flag
    CpuTimer0Regs.TCR.bit.TIF = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    // This is an example of the aforementioned registers with multiple flag bits
    // in which writing a 1 clears that flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

    //TODO: calculate speed and convert to the range of +/- 1.0
    //TODO: read external saturation variable

    // external clamp for anti-windup reset
    if(DCL_runClamp_C1(&saturation, 1.0f, 0.0f)) // converts saturation to 1.0f or 0.0f
    {
        while(1); //TODO: replace with error handling/logging if saturation occurs
    }

    // run PID controller
    control_output = DCL_runPID_C4(&pid_controller, reference, feedback, saturation); // if saturation variable not used, make it a 1.0f

    //TODO: convert control_output to PWM duty cycle and set new motor speed

}

__interrupt void cpu_timer1_isr(void)
{
    GpioDataRegs.GPBTOGGLE.bit.GPIO52 = 1;

    // Clear timer int flag
    CpuTimer1Regs.TCR.bit.TIF = 1;

    // You don't have to do the ACK for PIE here since timer1 uses its own
    // interrupt line for whatever reason...
}

__interrupt void epwm1_isr(void)
{
    // Do nothing

    //
    // Clear INT flag for this timer
    //
    EPwm1Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm2_isr(void)
{
    // Do nothing

    //
    // Clear interrupt flag
    //
    EPwm2Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void epwm3_isr(void)
{
    // Do nothing

    //
    // Clear interrupt flag
    //
    EPwm3Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

__interrupt void adc_isr(void)
{
    //
    // Clear ADCINT1 flag
    //
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

static void configure_ePWM(void)
{
    // Stop all TB (Time-Base) Clocks
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    // EPWM Module 1 config
    EPwm1Regs.TBPRD = 800; // Period = 1600 TBCLK counts
    EPwm1Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Symmetrical mode
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Master module
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Sync down-stream module
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET; // set actions for EPWM1A
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; // enable Dead-band module
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; // Active Hi complementary
    EPwm1Regs.DBFED = 50; // FED = 50 TBCLKs
    EPwm1Regs.DBRED = 50; // RED = 50 TBCLKs

    // EPWM Module 2 config
    EPwm2Regs.TBPRD = 800; // Period = 1600 TBCLK counts
    EPwm2Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Symmetrical mode
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE; // Slave module
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm2Regs.AQCTLA.bit.CAU = AQ_SET; // set actions for EPWM2A
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; // enable Dead-band module
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; // Active Hi complementary
    EPwm2Regs.DBFED = 50; // FED = 50 TBCLKs
    EPwm2Regs.DBRED = 50; // RED = 50 TBCLKs

    // EPWM Module 3 config
    EPwm3Regs.TBPRD = 800; // Period = 1600 TBCLK counts
    EPwm3Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Symmetrical mode
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE; // Slave module
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm3Regs.AQCTLA.bit.CAU = AQ_SET; // set actions for EPWM3A
    EPwm3Regs.AQCTLA.bit.CAD = AQ_CLEAR;
    EPwm3Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; // enable Dead-band module
    EPwm3Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC; // Active Hi complementary
    EPwm3Regs.DBFED = 50; // FED = 50 TBCLKs
    EPwm3Regs.DBRED = 50; // RED = 50 TBCLKs

    // Duty Cycles for each ePWM
    EPwm1Regs.CMPA.half.CMPA = 400;
    EPwm2Regs.CMPA.half.CMPA = 400;
    EPwm3Regs.CMPA.half.CMPA = 400;

    // Restart the TB Clocks
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

static void configure_ADC(void)
{
    //
    // Configure ADC
    //
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

    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 4;  // set SOC0 channel select to ADCINA4
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 2;  // set SOC1 channel select to ADCINA2

    //
    // set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts
    // first then SOC1
    //
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 5;

    //
    // set SOC1 start trigger on EPWM1A, due to round-robin SOC0 converts
    // first then SOC1
    //
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;

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
