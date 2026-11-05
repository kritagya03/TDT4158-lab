#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

/**
 * @file main.c
 * @author krita
 * @date 2026-10-30
 * @brief Main function
 */

// Configure voltage reference
void VREF_init(void) {
    VREF.ACREF = VREF_REFSEL_1V024_gc;
}

// Initialize Analog Comparator
void AC_init(void) {
    PORTD.DIRCLR = PIN2_bm;
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc
                | AC_MUXNEG_DACREF_gc;

    AC0.DACREF = 25;

    AC0.CTRLA = AC_ENABLE_bm | AC_RUNSTDBY_bm;

    AC0.INTCTRL = 0;
}

// Configure LED pin
void LED_init(void) {
    PORTA.DIRSET = PIN2_bm;
}

// Configure Event System
void EVSYS_init(void) {
    EVSYS.CHANNEL0 = EVSYS_CHANNEL0_AC0_OUT_gc;

    EVSYS.USEREVSYSEVOUTA  = 1; 

    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUTA_bm;
}

int main(void) {
    VREF_init();
    AC_init();
    LED_init();
    EVSYS_init();

    // Put CPU into Standby sleep mode
    set_sleep_mode(SLEEP_MODE_STANDBY);
    sleep_enable();


    while (1) {
        sleep_cpu();
    }

    return 0;
}