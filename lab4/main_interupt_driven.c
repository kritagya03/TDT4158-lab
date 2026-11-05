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

void AC_init(void) {
    PORTD.DIRCLR = PIN2_bm;
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    // Positive input: PD2 (AINP0)
    // Negative input: DACREF
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc
                | AC_MUXNEG_DACREF_gc;

    // Threshold = 0.1 V
    AC0.DACREF = 25;

    // Enable comparator and allow it to run in Standby
    AC0.CTRLA = AC_ENABLE_bm | AC_RUNSTDBY_bm;

    AC0.INTCTRL = AC_CMP_bm;
}

void VREF_init(void) {
    VREF.ACREF = VREF_REFSEL_1V024_gc;
}

bool AC_above_threshold(void) {
    if (AC0.STATUS & AC_CMPSTATE_bm) {
        return true;
    } else {
        return false;
    }
}

void LED_init(void) {
    PORTA.OUTSET = PIN2_bm; // Start with LED OFF
    PORTA.DIRSET = PIN2_bm;
}

void set_LED_on(void) {
    PORTA.OUTCLR = PIN2_bm;
}

void set_LED_off(void) {
    PORTA.OUTSET = PIN2_bm;
}

ISR(AC0_AC_vect) {
    // Update LED based on comparator output
    if (AC_above_threshold()) {
        set_LED_off(); 
    } else {
        set_LED_on();  
    }

    // Clear comparator interrupt flag
    AC0.STATUS = AC_CMPIF_bm;
}

int main(void) {
    VREF_init();
    AC_init();
    LED_init();

    if (AC_above_threshold()) {
        set_LED_off();
    } else {
        set_LED_on();
    }

    set_sleep_mode(SLEEP_MODE_STANDBY);
    sleep_enable();

    // Enable global interrupts
    sei();

    while (1) {
        // Sleep until an analog comparator interrupt
        sleep_cpu();
    }

    return 0;
}