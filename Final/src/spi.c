/*
 * spi.c
 *
 *  Created on: Feb 13, 2020
 *      Author: sonya
 *
 *      Silk15 = P16 = GPIO16 = SIMO_A (SDI)
 *      Silk14 = P17 = GPIO17 = SOMI_A (SDO)
 *      Silk7 = P18 = GPIO18 = SCLK_A
 *      Silk19 = P19 = GPIO19 = CS_A (STE)
 */

//#include "DSP28x_Project.h"
#include "inc/spi.h"


Uint16 sdata;     // Send data buffer
Uint16 rdata;     // Receive data buffer
Uint16 rx_int_cnt;
Uint16 tx_int_cnt;


__interrupt void spiTxFifoIsr(void);
__interrupt void spiRxFifoIsr(void);

void spi_init(void)
{
    //Uint16 i;

    EALLOW;    //  write to EALLOW protected registers
    PieVectTable.SPIRXINTA = &spiRxFifoIsr;
    PieVectTable.SPITXINTA = &spiTxFifoIsr;
    EDIS;      // disable write to EALLOW protected registers

    SpiaRegs.SPICCR.bit.SPISWRESET=0; // Reset SPI

    SpiaRegs.SPICCR.all=0x000F;       // 16-bit (SPI_BUFFER_SZ) character, Loopback mode disabled
    SpiaRegs.SPICTL.all=0x0017;       // Overrun interrupt enabled, Master/Slave XMIT enabled
    SpiaRegs.SPISTS.all=0x0000;
    SpiaRegs.SPIBRR=0x0063;           // Baud rate
    SpiaRegs.SPIFFTX.all=0xC001;      // Disable FIFO TX int, set TX FIFO level to 1
    SpiaRegs.SPIFFRX.all=0x0021;      // Enable RX int, Set RX FIFO level to 1
    SpiaRegs.SPIFFCT.all=0x00;
    SpiaRegs.SPIPRI.all=0x0010;

    SpiaRegs.SPICCR.bit.SPISWRESET=1;  // Enable SPI

    SpiaRegs.SPIFFTX.bit.TXFIFO=1;
    SpiaRegs.SPIFFRX.bit.RXFIFORESET=1;

    sdata = 0;
    rdata = 0xBEEF;

    rx_int_cnt = 0;
    tx_int_cnt = 0;

    //
    // Enable interrupts required for this example
    //
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER6.bit.INTx1=1;     // Enable PIE Group 6, INT 1
    PieCtrlRegs.PIEIER6.bit.INTx2=1;     // Enable PIE Group 6, INT 2
    IER |= M_INT6;                       // Enable CPU INT6

}


//
// Some Useful local functions
//
void delay_loop()
{
    long      i;
    for (i = 0; i < 1000000; i++)
    {

    }
}

//
// error -
//
void error(void)
{
    __asm("     ESTOP0");  //Test failed!! Stop!
    for (;;);
}

void send_spi_control_word(Uint16 r_w, Uint16 addr, Uint16 data)
{
    sdata = (Uint16)(((r_w & 0x0001) << 15) | ((addr & 0x000F) << 11) | (data & 0x07FF)); //(r_w)(addr)(addr)(addr)(addr)(data)...(data)
    SpiaRegs.SPIFFTX.bit.TXFFIENA = 1; // enable tx interrupt
}

Uint16 process_spi_response_word(Uint16 slave_response)
{
    return (slave_response & 0x07FFF); //top 5 bits of response from motor driver are don't cares
}





__interrupt void spiTxFifoIsr(void)
{
//    if(SpiaRegs.SPISTS.bit.INT_FLAG == 1)
//    {
        SpiaRegs.SPITXBUF = sdata;      // Send data

        SpiaRegs.SPIFFTX.bit.TXFFIENA = 0; //disable TX interrupt

        tx_int_cnt++;
//    }

    SpiaRegs.SPIFFTX.bit.TXFFINTCLR=1;  // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ACK
}


__interrupt void spiRxFifoIsr(void)
{
//    if(SpiaRegs.SPISTS.bit.INT_FLAG == 1)
//    {
        rdata = SpiaRegs.SPIRXBUF;     // Read data
        rdata = process_spi_response_word(rdata); //Process data

        rx_int_cnt++;
//    }

    SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1;  // Clear Overflow flag
    SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1;  // Clear Interrupt flag
    PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ack
}




