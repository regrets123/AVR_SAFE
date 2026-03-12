#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "safe.h"
#include "millis.h"
#include "servo.h"
#include "adc.h"

#define F_CPU 16000000UL
#define BAUD  9600
#define UBRR_VAL (F_CPU / (16UL * BAUD) - 1)

static void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)(UBRR_VAL);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8-N-1 */
}

int main(void) {
    uart_init();
    uart_puts("BOOT\n");
    millis_init();
    init_servo();
    sei();
    lock();
    adc_init();
    srand(adc_read(3)); // seed with A3 floating noise

    // PD2, PD3, PD4 as outputs for LEDs
    BIT_SET(DDRD, PD2);
    BIT_SET(DDRD, PD3);
    BIT_SET(DDRD, PD4);

    uint16_t inputs[3];
    uint16_t goalValues[3];
    for (uint8_t i = 0; i < 3; i++)
    {
        goalValues[i] = rand() % 1024; // 0–1023, matches ADC range
    }

    for (uint8_t i = 0; i < 3; i++) {
        uart_puts("goal A");
        uart_putchar('0' + i);
        uart_puts(": ");
        uart_putuint(goalValues[i]);
        uart_putchar('\n');
    }

    uint8_t prevLockState = 0;
    millis_t lastTick = millis();

    while (1) {
        if (millis() - lastTick < 333)
            continue;
        lastTick = millis();

        check_inputs(inputs, goalValues);
        update_leds(inputs, prevLockState);
        if (inputLockState == UNLOCKED && prevLockState != UNLOCKED)
            unlock();
        else if (inputLockState != UNLOCKED && prevLockState == UNLOCKED)
            lock();

        prevLockState = inputLockState;
    }
    return 0;
}
