/*
 * drv8305_config.h
 *
 *  Created on: Feb 1, 2020
 *      Author: gufu
 */

#ifndef INC_DRV8305_CONFIG_H_
#define INC_DRV8305_CONFIG_H_

#include <stdbool.h>
#include "DSP28x_Project.h"


#define SPI_CMD_SZ 5
#define SPI_DATA_SZ 11
#define SPI_BUFFER_SZ (SPI_CMD_SZ + SPI_DATA_SZ)

#define SPI_READ 1
#define SPI_WRITE 0

#define SPI_REG_ADDR_WARN_WATCHDOG 0x1
#define SPI_REG_ADDR_OV_VDS_FAULTS 0x2
#define SPI_REG_ADDR_IC_FAULTS 0x3
#define SPI_REG_ADDR_VGS_FAULTS 0x4
#define SPI_REG_ADDR_HS_GATE_DRV_CTRL 0x5
#define SPI_REG_ADDR_LS_GATE_DRV_CTRL 0x6
#define SPI_REG_ADDR_GATE_DRV_CTRL 0x7
#define SPI_REG_ADDR_IC_OPERATION 0x9
#define SPI_REG_ADDR_SHUNT_AMP_CTRL 0xA
#define SPI_REG_ADDR_VOLT_REG_CTRL 0xB
#define SPI_REG_ADDR_VDS_SENSE_CTRL 0xC

#define WARN_WATCHDOG_RSVD_MASK 0x2FF//(0s are reserved spots)
#define OV_VDS_FAULTS_RSVD_MASK 0x7D7
#define IC_FAULTS_RSVD_MASK 0x777
#define VGS_FAULTS_RSVD_MASK 0x7E0
#define HS_GATE_DRV_CTRL_RSVD_MASK 0x3FF
#define LS_GATE_DRV_CTRL_RSVD_MASK 0x3FF
#define GATE_DRV_CTRL_RSVD_MASK 0x3FF
#define IC_OPERATION_RSVD_MASK 0x7FF
#define SHUNT_AMP_CTRL_RSVD_MASK 0x7FF
#define VOLT_REG_CTRL_RSVD_MASK 0x31F
#define VDS_SENSE_CTRL_RSVD_MASK 0x0FF

typedef struct {
    // fault regs
    Uint16 warnings;
    Uint16 ov_vds_faults;
    Uint16 ic_faults;
    Uint16 vgs_faults;
} drv8305_fault_regs_t;





void drv8305_init(void);

void enable_drv8305(void);

bool fault_cleared(void);

//-----------------------------

bool initialize_drv8305(void);
void spi_init(void);
Uint16 read_drv8305_reg(Uint16 address);
bool read_drv8305_fault_regs(drv8305_fault_regs_t * faults);
void handle_drv8305_faults(drv8305_fault_regs_t * faults);
bool write_drv8305_reg(Uint16 address, Uint16 data, Uint16 rsvd_mask);

void test_read_and_write(void);




#endif /* INC_DRV8305_CONFIG_H_ */
