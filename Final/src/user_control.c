/*
 * user_control.c
 *
 *  Created on: Mar 1, 2020
 *      Author: sonya
 */


#include "DSP28x_Project.h"
#include "inc/user_control.h"
#include "inc/command_queue.h"

#ifdef FLASH_MODE
#pragma CODE_SECTION(scibTxFifoIsr, "ramfuncs");
#pragma CODE_SECTION(scibRxFifoIsr, "ramfuncs");
#endif

__interrupt void scibTxFifoIsr(void);
__interrupt void scibRxFifoIsr(void);

extern command_queue_t command_q;

//
// scib_fifo_init -
//
static void scib_fifo_init(void)
{
    ScibRegs.SCIFFTX.all=0xE000;
    ScibRegs.SCIFFRX.all=0x2041;
    ScibRegs.SCIFFCT.all=0x0;

    //
    // 1 stop bit,  No loopback, No parity,8 char bits, async mode,
    // idle-line protocol
    //
    ScibRegs.SCICCR.all =0x0007;

    //
    // enable TX, RX, internal SCICLK, Disable RX ERR, SLEEP, TXWAKE
    //
    ScibRegs.SCICTL1.all =0x0003;

    //
    // disable TX, enable RX interrupts
    //
    ScibRegs.SCICTL2.bit.TXINTENA = 0;
    //ScibRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScibRegs.SCIFFRX.bit.RXFFIENA = 1;

    //
    // 9600 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK)
    //
    ScibRegs.SCIHBAUD    =0x0001;
    ScibRegs.SCILBAUD    =0x0024;

    ScibRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
    ScibRegs.SCIFFRX.bit.RXFIFORESET=1;
}

void usr_ctrl_send_msg(Uint16 data)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0);
    ScibRegs.SCITXBUF = data;
}

void usr_ctrl_comm_init(void)
{
    //
    // Initialize SCI-B GPIO pins
    // NOTE: make sure JP6 and JP7 on the board are both ON to send UART to PC
    //
    InitScibGpio();

    //
    // Remap Interrupts to ISR functions found within this file
    //
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.SCIRXINTB = &scibRxFifoIsr;
    EDIS;   // This is needed to disable write to EALLOW protected registers

    scib_fifo_init();

    //
    // Enable interrupts
    //
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx3=1;     // Interrupt enable for PIE Group 9, INT3 = SCIRXINTB
    IER |= 0x100;                        // Enable CPU INT Group 9
}

//
// scibRxFifoIsr -
//
__interrupt void scibRxFifoIsr(void)
{
    Uint16 rdataB = ScibRegs.SCIRXBUF.all;  // Read data

    commands_e received = rdataB & 0xFF;

    if ((received == STOP) ||
        (received == BRING_ME_DOWN) ||
        (received == PULL_ME_UP))
    {
        queue_push(&command_q, received);
    }

    //TODO: push rdataB_uart to FIFO buffer
    //TODO: command error checking?

    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1;   // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= 0x100;       // Issue PIE ack
}





