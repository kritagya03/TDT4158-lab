#include "usart.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/**
 * @file main.c
 * @author krita
 * @date 2026-10-30
 * @brief Main function
 */


void AC_init(void) {
    // Configure PD2 as an analog input
    PORTD.DIRCLR = PIN2_bm;
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

    // Positive input: PD2 (AINP0)
    // Negative input: DACREF
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

    // Set threshold to approximately 0.1 V
    AC0.DACREF = 25;

    // Enable analog comparator register
    AC0.CTRLA = AC_ENABLE_bm;
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
    PORTA.OUTSET = PIN2_bm; // Start with LED off
    PORTA.DIRSET = PIN2_bm;
}

void set_LED_on(void) {
    PORTA.OUTCLR = PIN2_bm;
}

void set_LED_off(void) {
    PORTA.OUTSET = PIN2_bm;
}

void TCA0_init(void) {
    // 10 ms timer period
    TCA0.SINGLE.PER = 20000;

    // Enable timer overflow interrupt
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;

    // Timer runs in standby, prescaler = 2, enable
    TCA0.SINGLE.CTRLA =
        TCA_SINGLE_RUNSTDBY_bm |
        TCA_SINGLE_CLKSEL_DIV2_gc |
        TCA_SINGLE_ENABLE_bm;
}

ISR(TCA0_OVF_vect) {
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

int main(void) {
    VREF_init();
    AC_init();
    TCA0_init();
    LED_init(); 

    set_sleep_mode(SLEEP_MODE_STANDBY);
    sleep_enable();

    //enable global interrupts
    sei();

    while (1) {
        if (AC_above_threshold()) {
            set_LED_off();
        } else {
            set_LED_on();
        }

        sleep_cpu();
    }

    return 0;
}