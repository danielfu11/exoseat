/*
 * debug.c
 *
 *  Created on: Mar 1, 2020
 *      Author: sonya
 */

#include "DSP28x_Project.h"
#include "inc/debug.h"


//
// scia_fifo_init - Initalize the SCI FIFO
//
 static void scib_fifo_init(void)
{
    ScibRegs.SCIFFTX.all=0xE040;
    ScibRegs.SCIFFRX.all=0x2044;
    ScibRegs.SCIFFCT.all=0x0;
}

 //
 // scia_xmit - Transmit a character from the SCI
 //
 static void scib_xmit(int a)
 {
     while (ScibRegs.SCIFFTX.bit.TXFFST != 0);
     ScibRegs.SCITXBUF = a;
 }


//
// debug_comm_init - SCIA  DLB, 8-bit word, baud rate 0x0103,
// default, 1 STOP bit, no parity
//
void debug_comm_init(void)
{
    //
    // Initialize SCI-A GPIO pins
    // NOTE: make sure JP6 and JP7 on the board are both ON to send UART to PC
    //
    InitScibGpio();

    //
    // Note: Clocks were turned on to the SCIA peripheral
    // in the InitSysCtrl() function
    //
    scib_fifo_init();
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
    // disable interrupts
    //
    ScibRegs.SCICTL2.bit.TXINTENA = 0;
    ScibRegs.SCICTL2.bit.RXBKINTENA = 0;

    //
    // 9600 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK)
    //
    ScibRegs.SCIHBAUD    =0x0001;
    ScibRegs.SCILBAUD    =0x0024;

    ScibRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
}


void debug_send_msg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        scib_xmit(msg[i]);
        i++;
    }
}

void debug_send_float(float data)
{
    Uint16 integer = (Uint16)data;
    Uint16 decimal = (data - integer)*1000;

    scib_xmit(((integer % 1000)/100)+48);   //decimal ascii val for hundreds place (because integer division always rounds down so number/100 will be 1 if number = 145, and 48 is the decimal offset for ascii numbers (ascii 0 is decimal 48)
    scib_xmit(((integer % 100)/10)+48);     //decimal ascii val for tens place (because number % 100 will be between 0 and 99 so it doesnt care about hundreds place
    scib_xmit(((integer % 10)/1)+48);       //decimal ascii val for ones place (number % 10 will only care about ones place)
    scib_xmit(0x2E);                        //decimal point
    scib_xmit(((decimal % 1000)/100)+48);
    scib_xmit(((decimal % 100)/10)+48);
    scib_xmit(((decimal % 10)/1)+48);
}

void debug_send_int(Uint16 integer)
{
    scib_xmit(((integer % 100000)/10000)+48);
    scib_xmit(((integer % 10000)/1000)+48);
    scib_xmit(((integer % 1000)/100)+48);   //decimal ascii val for hundreds place (because integer division always rounds down so number/100 will be 1 if number = 145, and 48 is the decimal offset for ascii numbers (ascii 0 is decimal 48)
    scib_xmit(((integer % 100)/10)+48);     //decimal ascii val for tens place (because number % 100 will be between 0 and 99 so it doesnt care about hundreds place
    scib_xmit(((integer % 10)/1)+48);       //decimal ascii val for ones place (number % 10 will only care about ones place)
}

