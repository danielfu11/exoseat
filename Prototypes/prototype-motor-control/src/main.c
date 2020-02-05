
#include "DSP28x_Project.h"

#include "inc/hallsensor.h"
#include "inc/commutation.h"
#include "inc/drv8305_config.h"
#include "inc/timer.h"

extern volatile bool new_hall_state;

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

    // Initialize timer
    timer_init();

    // Initialize hall sensors
    hallsensor_init();

    // Initialize gate drive pins
    gate_drive_init();

    // Initialize DRV8305
    drv8305_init();

    // Enable global interrupts
    EINT;

    // Enable DRV8305 if nFault
    enable_drv8305();
    delay_1ms();

    // Read initial hall sensor states
    Uint8 hall_state = read_hall_states();

    // Commutate
    phase_drive_s drive_state = next_commutation_state(CW, hall_state, true);

    while (1)
    {
        if (fault_cleared())
        {
            if (new_hall_state)
            {
                hall_state = read_hall_states();
                drive_state = next_commutation_state(CW, hall_state, false);
                new_hall_state = false;
            }
        }
        else
        {
            enable_drv8305();
            delay_1ms();
        }
    }
}
