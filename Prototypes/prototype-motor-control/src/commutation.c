/*
 * commutation.c
 *
 *  Created on: Feb 2, 2020
 *      Author: gufu
 */

#include "inc/commutation.h"
#include "inc/speed_control.h"


#define INDEPENDENT_PWM    1
#define COMPLEMENTARY_PWM  0

volatile Uint32 duty_cycle = INITIAL_REFERENCE_SPEED;

__interrupt void epwm1_isr(void);
__interrupt void epwm2_isr(void);
__interrupt void epwm3_isr(void);

static inline void U_LOW_SIDE_ON(void)
{
    GpioDataRegs.GPASET.bit.GPIO1 = 1;
}

static inline void U_LOW_SIDE_OFF(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
}

static inline void V_LOW_SIDE_ON(void)
{
    GpioDataRegs.GPASET.bit.GPIO3 = 1;
}

static inline void V_LOW_SIDE_OFF(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
}

static inline void W_LOW_SIDE_ON(void)
{
    GpioDataRegs.GPASET.bit.GPIO5 = 1;
}

static inline void W_LOW_SIDE_OFF(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;
}

// Set up EPWM1A, 2A, 3A and set 1B 2B 3B pins as normal GPIO output pin for now
void gate_drive_init(void)
{
#if INDEPENDENT_PWM
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;    // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 0;    // Disable pull-up on GPIO2 (EPWM2A)
    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 0;    // Disable pull-up on GPIO4 (EPWM3A)

    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;   // Configure GPIO4 as EPWM3A

    // Configure as GPIO
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;

    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;

    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;
    EDIS;

#endif

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;

    EPwm1Regs.TBPRD = 2000;
    EPwm1Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Master module
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Sync down-stream module
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET; // set actions for EPWM1A
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    //
    // Interrupt where we will change the Compare Values
    //
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTRU_CMPA;     // Select INT on CMPA
    EPwm1Regs.ETSEL.bit.INTEN = 0;                // Disable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;           // Generate INT on 1st event

    // EPWM Module 2 config
    EPwm2Regs.TBPRD = 2000;
    EPwm2Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE; // Slave module
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET; // set actions for EPWM2A
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;

    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTRU_CMPA;     // Select INT on CMPA
    EPwm2Regs.ETSEL.bit.INTEN = 0;                // Disable INT
    EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;           // Generate INT on 1st event

    // EPWM Module 3 config
    EPwm3Regs.TBPRD = 2000;
    EPwm3Regs.TBPHS.half.TBPHS = 0; // Set Phase register to zero
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE; // Slave module
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // sync flow-through
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // load on CTR=Zero
    EPwm3Regs.AQCTLA.bit.ZRO = AQ_SET; // set actions for EPWM3A
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;

    EPwm3Regs.ETSEL.bit.INTSEL = ET_CTRU_CMPA;     // Select INT on CMPA
    EPwm3Regs.ETSEL.bit.INTEN = 0;                // Disable INT
    EPwm3Regs.ETPS.bit.INTPRD = ET_1ST;           // Generate INT on 1st event

    EPwm1Regs.CMPA.half.CMPA = 0;
    EPwm2Regs.CMPA.half.CMPA = 0;
    EPwm3Regs.CMPA.half.CMPA = 0;

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    // Map ePWM ISRs in PIE Vector Table
    EALLOW;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    PieVectTable.EPWM2_INT = &epwm2_isr;
    PieVectTable.EPWM3_INT = &epwm3_isr;
    EDIS;

    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;
    PieCtrlRegs.PIEIER3.bit.INTx3 = 1;

    IER |= M_INT3;
}

phase_drive_s next_commutation_state(direction_e dir, Uint8 hall_state, bool startup)
{
    phase_drive_s drive_state;
    if (CW == dir)
    {
        switch (hall_state)
        {
            // Hu=1 Hv=1 Hw=0, Pu=off Pv=- Pw=+
            case 0b110:
                drive_state.phase_u = NC;
                drive_state.phase_v = NEG;
                drive_state.phase_w = POS;
                if (startup)
                {
                    V_LOW_SIDE_ON();
                    EPwm3Regs.CMPA.half.CMPA = duty_cycle;
                }
                else
                {
                    // Enable interrupts for U and W ePWM
                    // to toggle PWM duty cycles
                    EPwm1Regs.CMPA.half.CMPA = 0;
                    EPwm3Regs.CMPA.half.CMPA = duty_cycle;
//                    state_change = 1;
//                    EPwm1Regs.ETSEL.bit.INTEN = 1;
//                    EPwm3Regs.ETSEL.bit.INTEN = 1;
                }
                break;

            case 0b100:
                drive_state.phase_u = NEG;
                drive_state.phase_v = NC;
                drive_state.phase_w = POS;
                if (startup)
                {
                    U_LOW_SIDE_ON();
                    EPwm3Regs.CMPA.half.CMPA = duty_cycle;
                }
                else
                {
                    U_LOW_SIDE_ON();
                    V_LOW_SIDE_OFF();
                }
                break;

            case 0b101:
                drive_state.phase_u = NEG;
                drive_state.phase_v = POS;
                drive_state.phase_w = NC;
                if (startup)
                {
                    U_LOW_SIDE_ON();
                    EPwm2Regs.CMPA.half.CMPA = duty_cycle;
                }
                else
                {
                    EPwm2Regs.CMPA.half.CMPA = duty_cycle;
                    EPwm3Regs.CMPA.half.CMPA = 0;
                }
                break;

            case 0b001:
                drive_state.phase_u = NC;
                drive_state.phase_v = POS;
                drive_state.phase_w = NEG;
                if (startup)
                {
                    W_LOW_SIDE_ON();
                    EPwm2Regs.CMPA.half.CMPA = duty_cycle;
                }
                else
                {
                    U_LOW_SIDE_OFF();
                    W_LOW_SIDE_ON();
                }
                break;

            case 0b011:
                drive_state.phase_u = POS;
                drive_state.phase_v = NC;
                drive_state.phase_w = NEG;
                if (startup)
                {
                    W_LOW_SIDE_ON();
                    EPwm1Regs.CMPA.half.CMPA = duty_cycle;
                }
                else
                {
                    EPwm1Regs.CMPA.half.CMPA = duty_cycle;
                    EPwm2Regs.CMPA.half.CMPA = 0;
                }
                break;

            case 0b010:
                drive_state.phase_u = POS;
                drive_state.phase_v = NEG;
                drive_state.phase_w = NC;
                if (startup)
                {
                    V_LOW_SIDE_ON();
                    EPwm1Regs.CMPA.half.CMPA = duty_cycle;
                }
                else
                {
                    W_LOW_SIDE_OFF();
                    V_LOW_SIDE_ON();
                }
                break;

            default:
                // Fault
                break;
        }

    }
    else if (CCW == dir)
    {

    }
    else
    {
        // Fault
    }
    return drive_state;
}

__interrupt void epwm1_isr(void)
{
    EPwm1Regs.ETSEL.bit.INTEN = 0;
    EPwm1Regs.ETCLR.bit.INT = 1;
    if (EPwm1Regs.CMPA.half.CMPA != 0 && EPwm1Regs.CMPA.half.CMPA != duty_cycle)
    {
        EPwm1Regs.CMPA.half.CMPA = duty_cycle;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}
__interrupt void epwm2_isr(void)
{
    EPwm2Regs.ETSEL.bit.INTEN = 0;
    EPwm2Regs.ETCLR.bit.INT = 1;
    if (EPwm2Regs.CMPA.half.CMPA != 0 && EPwm2Regs.CMPA.half.CMPA != duty_cycle)
    {
        EPwm2Regs.CMPA.half.CMPA = duty_cycle;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}
__interrupt void epwm3_isr(void)
{
    EPwm3Regs.ETSEL.bit.INTEN = 0;
    EPwm3Regs.ETCLR.bit.INT = 1;
    if (EPwm3Regs.CMPA.half.CMPA != 0 && EPwm3Regs.CMPA.half.CMPA != duty_cycle)
    {
        EPwm3Regs.CMPA.half.CMPA = duty_cycle;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

