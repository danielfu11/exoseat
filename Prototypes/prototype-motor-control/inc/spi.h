/*
 * spi.h
 *
 *  Created on: Feb 13, 2020
 *      Author: sonya
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "DSP28x_Project.h"

#define SPI_CMD_SZ 5
#define SPI_DATA_SZ 11
#define SPI_BUFFER_SZ (SPI_CMD_SZ + SPI_DATA_SZ)

#define SPI_READ 1
#define SPI_WRITE 0

#define SPI_REG_ADDR_WARN_WATCH 1



void spi_init(void);
void send_spi_control_word(Uint16 r_w, Uint16 address, Uint16 data);
Uint16 process_spi_response_word(Uint16 slave_response);


#endif /* INC_SPI_H_ */
