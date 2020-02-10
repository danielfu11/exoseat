/*
 * hallsensor.c
 *
 *  Created on: Feb 1, 2020
 *      Author: gufu
 */

#include "inc/hallsensor.h"

volatile bool new_hall_state = false;
volatile Uint32 ticks_moved = 0; // 1 tick == 30 deg

static volatile Uint32 hall_tmr_prev = 0;
static volatile Uint32 hall_tmr_cur = 0;

// ISR prototypes
__interrupt void xint1_isr(void);
__interrupt void xint2_isr(void);
__interrupt void xint3_isr(void);

static void xint_unified_isr(void)
{
    new_hall_state = true;
    ticks_moved++;
    hall_tmr_prev = hall_tmr_cur;
    hall_tmr_cur = CpuTimer1.InterruptCount;
}

void hallsensor_init(void)
{
    // U: GPIO6, V: GPIO7, W: GPIO8
    EALLOW;

    // Enable pull up
    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 0;

    // Input sync'd to SYSCLKOUT
    GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;

    // Select GPIO function
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;

    // Set pins to input
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;

    // Set GPIO6 to interrupt on XINT1, GPIO7->XINT2, GPIO8
    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 6;
    GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL = 7;
    GpioIntRegs.GPIOXINT3SEL.bit.GPIOSEL = 8;

    // Map ISRs in vector table
    PieVectTable.XINT1 = &xint1_isr;
    PieVectTable.XINT2 = &xint2_isr;
    PieVectTable.XINT3 = &xint3_isr;

    EDIS;

    // XINT1 INT1.4, XINT2 INT1.5, XINT3 INT12.1
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1;
    PieCtrlRegs.PIEIER12.bit.INTx1 = 1;

    // Configure interrupts to trigger on both rising and falling
    XIntruptRegs.XINT1CR.bit.POLARITY = 0b11;
    XIntruptRegs.XINT2CR.bit.POLARITY = 0b11;
    XIntruptRegs.XINT3CR.bit.POLARITY = 0b11;

    // Enable XINT interrupts
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;
    XIntruptRegs.XINT2CR.bit.ENABLE = 1;
    XIntruptRegs.XINT3CR.bit.ENABLE = 1;

    // Enable CPU INT1 and INT12
    IER |= M_INT1 | M_INT12;
}

Uint8 read_hall_states(void)
{
    Uint16 hall_u, hall_v, hall_w;
    hall_u = GpioDataRegs.GPADAT.bit.GPIO6;
    hall_v = GpioDataRegs.GPADAT.bit.GPIO7;
    hall_w = GpioDataRegs.GPADAT.bit.GPIO8;

    return (hall_u << 2) | (hall_v << 1) | hall_w;
}

Uint32 calculate_speed(void)
{
    Uint32 tick_time = hall_tmr_cur - hall_tmr_prev; // 1 uS per tick
    return (Uint32) (5000000 / tick_time); // 1000000 * 60 / (tick_time * 12) = RPM

}

__interrupt void xint1_isr(void)
{
    xint_unified_isr();

    // Acknowledge interrupt
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void xint2_isr(void)
{
    xint_unified_isr();

    // Acknowledge interrupt
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void xint3_isr(void)
{
    xint_unified_isr();

    // Acknowledge interrupt
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
