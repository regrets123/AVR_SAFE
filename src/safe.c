#include <avr/io.h>
#include "safe.h"
#include "servo.h"
#include "millis.h"
#include "adc.h"

uint8_t inputLockState = 0; /* all locked by default */

void uart_putchar(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_puts(const char *s) {
    while (*s) uart_putchar(*s++);
}

void uart_putuint(uint16_t val) {
    char buf[6];
    int8_t i = 0;
    if (val == 0) { uart_putchar('0'); return; }
    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (--i >= 0) uart_putchar(buf[i]);
}

void check_inputs(uint16_t *inputs, uint16_t *goalValues) {
    int range = 70;
    for (uint8_t i = 0; i < 3; i++) {
        inputs[i] = adc_read(i);
        uint16_t lower = goalValues[i] > range ? goalValues[i] - range : 0;
        uint16_t upper = goalValues[i] < 923 ? goalValues[i] + range : 1023;
        if (inputs[i] >= lower && inputs[i] <= upper)
            BIT_SET(inputLockState, i);
        else
            BIT_CLEAR(inputLockState, i);
    }
}

void update_leds(uint16_t *inputs, uint8_t prevLockState) {
    const uint8_t ledPins[3] = {PD2, PD3, PD4};
    for (uint8_t i = 0; i < 3; i++) {
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
}

uint8_t lightLed(uint8_t index)
{
    return 0;
}

uint8_t unlock()
{
    servo1_set_percentage(-60); //Full right
    return 0;
}

uint8_t lock()
{
    servo1_set_percentage(100); // Full left
    
    return 0;
}
