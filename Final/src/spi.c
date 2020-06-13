///*
// * spi.c
// *
// *  Created on: Feb 13, 2020
// *      Author: sonya
// *
// *      Silk15 = P16 = GPIO16 = SIMO_A (SDI)
// *      Silk14 = P17 = GPIO17 = SOMI_A (SDO)
// *      Silk7 = P18 = GPIO18 = SCLK_A
// *      Silk19 = P19 = GPIO19 = CS_A (STE)
// */
//
////#include "DSP28x_Project.h"
//#include "stdbool.h"
//#include "inc/spi.h"
//
//
//Uint16 sdata_spi;     // Send data buffer
//volatile Uint16 rdata_spi;     // Receive data buffer
//volatile Uint16 rx_int_cnt;
//volatile Uint16 tx_int_cnt;
//volatile Uint8 spi_done = 0;
//
//
//__interrupt void spiTxFifoIsr(void);
//__interrupt void spiRxFifoIsr(void);
//
//
////static Uint16 process_spi_response_word(Uint16 slave_response)
////{
////    return (slave_response & 0x07FFF); //top 5 bits of response from motor driver are don't cares
////}
//
//
//static inline void send_word_drv8305(Uint16 r_w, Uint16 addr, Uint16 data)
//{
//    sdata_spi = (Uint16)(((r_w & 0x0001) << 15) | ((addr & 0x000F) << 11) | (data & 0x07FF)); //(r_w)(addr)(addr)(addr)(addr)(data)...(data)
//    SpiaRegs.SPIFFTX.bit.TXFFIENA = 1; // enable tx interrupt
//}
//
//
//void spi_init(void)
//{
//    InitSpiaGpio();
//
//    EALLOW;    //  write to EALLOW protected registers
//    PieVectTable.SPIRXINTA = &spiRxFifoIsr;
//    PieVectTable.SPITXINTA = &spiTxFifoIsr;
//    EDIS;      // disable write to EALLOW protected registers
//
//    SpiaRegs.SPICCR.bit.SPISWRESET=0; // Reset SPI
//
//    SpiaRegs.SPICCR.all=0x000F;       // 16-bit (SPI_BUFFER_SZ) character, Loopback mode disabled
//    SpiaRegs.SPICTL.all=0x0017;       // Overrun interrupt enabled, Master/Slave XMIT enabled
//    SpiaRegs.SPISTS.all=0x0000;
//    SpiaRegs.SPIBRR=0x0063;           // Baud rate
//    SpiaRegs.SPIFFTX.all=0xC001;      // Disable FIFO TX int, set TX FIFO level to 1
//    SpiaRegs.SPIFFRX.all=0x0001;      // Disable RX int, Set RX FIFO level to 1
//    SpiaRegs.SPIFFCT.all=0x00;
//    SpiaRegs.SPIPRI.all=0x0010;
//
//    SpiaRegs.SPICCR.bit.SPISWRESET=1;  // Enable SPI
//
//    SpiaRegs.SPIFFTX.bit.TXFIFO=1;
//    SpiaRegs.SPIFFRX.bit.RXFIFORESET=1;
//
//    sdata_spi = 0;
//    rdata_spi = 0xBEEF;
//
//    rx_int_cnt = 0;
//    tx_int_cnt = 0;
//
//    //
//    // Enable interrupts required for this example
//    //
//    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
//    PieCtrlRegs.PIEIER6.bit.INTx1=1;     // Enable PIE Group 6, INT 1
//    PieCtrlRegs.PIEIER6.bit.INTx2=1;     // Enable PIE Group 6, INT 2
//    IER |= M_INT6;                       // Enable CPU INT6
//
//}
//
//
//Uint16 read_drv8305_reg(Uint16 address)
//{
//    Uint16 data_store = 1;
//
//    SpiaRegs.SPIFFRX.bit.RXFFIENA = 1; // enable rx interrupt
//
//    send_word_drv8305(SPI_READ, address, 0);
//    while(!spi_done);
//    data_store = rdata_spi;
//    spi_done = 0;
//
//    DELAY_US(100);
//
//    return data_store;
//}
//
//
//bool read_drv8305_fault_regs(drv8305_fault_regs_t * faults)
//{
//    bool status = true;
//
//    faults->warnings = read_drv8305_reg(SPI_REG_ADDR_WARN_WATCHDOG);
//    if (faults->warnings != 0x0000)
//    {
//        status = false;
//    }
//    faults->ov_vds_faults = read_drv8305_reg(SPI_REG_ADDR_OV_VDS_FAULTS);
//    if (faults->ov_vds_faults != 0x0000)
//    {
//        status = false;
//    }
//    faults->ic_faults = read_drv8305_reg(SPI_REG_ADDR_IC_FAULTS);
//    if (faults->ic_faults != 0x0000)
//    {
//        status = false;
//    }
//    faults->vgs_faults = read_drv8305_reg(SPI_REG_ADDR_VGS_FAULTS);
//    if (faults->vgs_faults != 0x0000)
//    {
//        status = false;
//    }
//
//    return status;
//}
//
//void handle_drv8305_faults(drv8305_fault_regs_t * faults)
//{
//    bool status = true;
//
//    faults->warnings = read_drv8305_reg(SPI_REG_ADDR_WARN_WATCHDOG);
//    if (faults->warnings != 0x0000)
//    {
//        status = false;
//    }
//    faults->ov_vds_faults = read_drv8305_reg(SPI_REG_ADDR_OV_VDS_FAULTS);
//    if (faults->ov_vds_faults != 0x0000)
//    {
//        status = false;
//    }
//    faults->ic_faults = read_drv8305_reg(SPI_REG_ADDR_IC_FAULTS);
//    if (faults->ic_faults != 0x0000)
//    {
//        status = false;
//    }
//    faults->vgs_faults = read_drv8305_reg(SPI_REG_ADDR_VGS_FAULTS);
//    if (faults->vgs_faults != 0x0000)
//    {
//        status = false;
//    }
//
//    return status;
//}
//
//bool write_drv8305_reg(Uint16 address, Uint16 data)
//{
//    bool status = true;
//    Uint16 read_check, read_check0; // variables for debugging
//
//    SpiaRegs.SPIFFRX.bit.RXFFIENA = 1; // enable rx interrupt
//
//    read_check0 = read_drv8305_reg(address);
//    send_word_drv8305(SPI_WRITE, address, data);
//
//    DELAY_US(100);
//    read_check = read_drv8305_reg(address);
//    read_check = read_drv8305_reg(address);
//    if(read_check != data) //check that data was written correctly
//    {
//        status = false;
//    }
//
////    read_check1 = read_drv8305_reg(address);
////    if(read_check1 != data) //check that data was written correctly
////    {
////        status = false;
////    }
////
////    read_check2 = read_drv8305_reg(0x6);
//
//    return status;
//}
//
//
//void test_read_and_write(void) //purely for debugging (using breakpoints)
//{
//    Uint16 drv8305_reg, drv8305_reg0, drv8305_reg1, drv8305_reg2, drv8305_reg3, drv8305_reg4, drv8305_reg5;
//
//    if(!write_drv8305_reg(SPI_REG_ADDR_HS_GATE_DRV_CTRL, 0x333))
//    {
//        //error
//    }
//
//    drv8305_reg = read_drv8305_reg(SPI_REG_ADDR_HS_GATE_DRV_CTRL);
//    drv8305_reg0 = read_drv8305_reg(SPI_REG_ADDR_LS_GATE_DRV_CTRL); // register read test
//    drv8305_reg1 = read_drv8305_reg(SPI_REG_ADDR_GATE_DRV_CTRL); // register read test
//    drv8305_reg2 = read_drv8305_reg(SPI_REG_ADDR_IC_OPERATION); // register read test
//    drv8305_reg3 = read_drv8305_reg(SPI_REG_ADDR_SHUNT_AMP_CTRL); // register read test
//    drv8305_reg4 = read_drv8305_reg(SPI_REG_ADDR_VOLT_REG_CTRL); // register read test
//    drv8305_reg5 = read_drv8305_reg(SPI_REG_ADDR_VDS_SENSE_CTRL); // register read test
//    drv8305_reg = read_drv8305_reg(SPI_REG_ADDR_HS_GATE_DRV_CTRL);
//}
//
//
//
//__interrupt void spiTxFifoIsr(void)
//{
////    if(SpiaRegs.SPISTS.bit.INT_FLAG == 1)
////    {
//        SpiaRegs.SPITXBUF = sdata_spi;      // Send data
//
//        SpiaRegs.SPIFFTX.bit.TXFFIENA = 0; //disable TX interrupt
//
//        tx_int_cnt++;
////    }
//
//    SpiaRegs.SPIFFTX.bit.TXFFINTCLR=1;  // Clear Interrupt flag
//    PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ACK
//}
//
//
//__interrupt void spiRxFifoIsr(void)
//{
////    if(SpiaRegs.SPISTS.bit.INT_FLAG == 1)
////    {
//    spi_done = 0;
//
//    rdata_spi = SpiaRegs.SPIRXBUF;     // Read data
//    //rdata_spi = process_spi_response_word(rdata_spi); //Process data
//
//    SpiaRegs.SPIFFRX.bit.RXFFIENA = 0; // disable rx interrupt
//
//    rx_int_cnt++;
////    }
//
//    SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1;  // Clear Overflow flag
//    SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1;  // Clear Interrupt flag
//    PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ack
//
//    spi_done = 1;
//}
