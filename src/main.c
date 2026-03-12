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

#define BIT_SET(reg, bit)    ((reg) |=  (1 << (bit)))
#define BIT_CLEAR(reg, bit)  ((reg) &= ~(1 << (bit)))
#define BIT_TOGGLE(reg, bit) ((reg) ^=  (1 << (bit)))
#define BIT_READ(reg, bit)   (((reg) >> (bit)) & 1)

static void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)(UBRR_VAL);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8-N-1 */
}

static void uart_putchar(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

static void uart_puts(const char *s) {
    while (*s) uart_putchar(*s++);
}

static void uart_putuint(uint16_t val) {
    char buf[6];
    int8_t i = 0;
    if (val == 0) { uart_putchar('0'); return; }
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (--i >= 0) uart_putchar(buf[i]);
}

int main(void) {
    uart_init();
    init_servo();
    sei();
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

    while (1) {
        for (uint8_t i = 0; i < 3; i++) {
            inputs[i] = adc_read(i); // A0, A1, A2
            uint16_t lower = goalValues[i] > 50 ? goalValues[i] - 50 : 0;
            uint16_t upper = goalValues[i] < 974 ? goalValues[i] + 50 : 1023;
            if (inputs[i] >= lower && inputs[i] <= upper){
                BIT_SET(inputLockState,i);
            }
            else
            {
                BIT_CLEAR(inputLockState,i);
            }
        }
        const uint8_t ledPins[3] = {PD2, PD3, PD4};
        for (uint8_t i = 0; i < 3; i++)
        {
            uint8_t isUnlocked = BIT_READ(inputLockState, i);
            uint8_t wasUnlocked = BIT_READ(prevLockState, i);

            if (isUnlocked)
                BIT_SET(PORTD, ledPins[i]);
            else
                BIT_CLEAR(PORTD, ledPins[i]);

            uart_puts("A");
            uart_putchar('0' + i);
            uart_puts(": ");
            uart_putuint(inputs[i]);
            uart_putchar(i < 2 ? ' ' : '\n');
        }
        prevLockState = inputLockState;
    }
    return 0;
}
