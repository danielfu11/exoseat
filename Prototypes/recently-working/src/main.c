
#include "DSP28x_Project.h"

#include "inc/hallsensor.h"
#include "inc/commutation.h"
#include "inc/drv8305_config.h"
#include "inc/timer.h"
#include "inc/speed_control.h"
#include "inc/spi.h"
#include "inc/debug.h"

#define ONE_REV     12

extern volatile bool new_hall_state;
extern volatile Uint32 ticks_moved;

float speed_arr[12];

DCL_PID pid_controller = PID_DEFAULTS;  //initialize before interrupts turn on
float reference;                        // controller set-point reference (rk)
float feedback;                         // measured feedback value (yk)
float saturation;                       // external output clamp flag (lk)
float control_output;                   // output of controller block (uk)

volatile Uint8 blah = 0;
Uint16 avg_speed;
extern Uint16 current_time;
//
//extern volatile Uint16 rdata_spi;
//extern volatile Uint8 spi_done;
//
//Uint16 ov_vds_fault, ic_fault, vgs_fault, test_reg, warn_watchdog;

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

    //Initialize SPI-A GPIO pins
    InitSpiaGpio();

    // Initialize timer
    timer_init();

    // Initialize hall sensors
    hallsensor_init();

    // Initialize gate drive pins
    gate_drive_init();

    // Initialize DRV8305
    drv8305_init();

    //Initialize PID speed controller
    controller_init();

    // Enable global interrupts
    EINT;

    // Enable DRV8305 if nFault
    enable_drv8305();
    delay_1ms();

    // Initialize SPI peripheral
    spi_init();

    //Debug comm init
    debug_comm_init();

    // Read initial hall sensor states
    Uint8 hall_state = read_hall_states();

    // Commutate
    phase_drive_s drive_state = next_commutation_state(CW, hall_state, true);


   // send_spi_control_word(SPI_READ, 0x5, 0);


    int i = 0;

    while (1)
    {
        if (fault_cleared())
        {
            if (new_hall_state)
            {
                feedback = (float) calculate_speed();
                speed_arr[i] = feedback;
                hall_state = read_hall_states();
                drive_state = next_commutation_state(CW, hall_state, false);
                new_hall_state = false;
                i++;
                //if (i == 12) i = 0;
                if (i == 2) blah = 1;
                if (i == 12)
                {
                    int j;
                    for (j=0; j < 12; j++)
                    {
                        avg_speed += speed_arr[j];
                    }
                    avg_speed = (Uint16) (avg_speed / 12);

//                    debug_send_int(current_time);
//                    debug_send_msg(",");
//                    debug_send_int(avg_speed);
//                    debug_send_msg(";");


                    i = 0;
                }
            }
        }
        else
        {
            enable_drv8305();
            delay_1ms();

            //SPI send/receive message test:
//            send_spi_control_word(SPI_READ, 0x5, 0); //NOTE: motor driver (and thus spi) needs 12V to work
//            while(!spi_done);
//            test_reg = rdata_spi;
//            spi_done = 0;

//            send_spi_control_word(SPI_READ, SPI_REG_ADDR_OV_VDS_FAULTS, 0);
//            while(!spi_done);
//            ov_vds_fault = rdata_spi;
//            spi_done = 0;
//
//            send_spi_control_word(SPI_READ, SPI_REG_ADDR_IC_FAULTS, 0);
//            while(!spi_done);
//            ic_fault = rdata_spi;
//            spi_done = 0;
//
//            send_spi_control_word(SPI_READ, SPI_REG_ADDR_VGS_FAULTS, 0);
//            while(!spi_done);
//            vgs_fault = rdata_spi;
//            spi_done = 0;
        }
    }
}
