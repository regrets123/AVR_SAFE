#ifndef SAFE_H_
#define SAFE_H_

#include <stdint.h>

#define BIT_SET(reg, bit)    ((reg) |=  (1 << (bit)))
#define BIT_CLEAR(reg, bit)  ((reg) &= ~(1 << (bit)))
#define BIT_TOGGLE(reg, bit) ((reg) ^=  (1 << (bit)))
#define BIT_READ(reg, bit)   (((reg) >> (bit)) & 1)
#define UNLOCKED             0x07

typedef enum { locked, unlocked, firstInput, secondInput, thirdInput, lastInput, idle } SafeState;

/* Bitmask tracking which analog inputs are unlocked.
 * Bit 0 = A0, bit 1 = A1, bit 2 = A2. A set bit means unlocked. */
extern uint8_t inputLockState;

void uart_putchar(char c);
void uart_puts(const char *s);
void uart_putuint(uint16_t val);

void check_inputs(uint16_t *inputs, uint16_t *goalValues);
void update_leds(uint16_t *inputs, uint8_t prevLockState);

uint8_t lightLed(uint8_t index);
uint8_t unlock();
uint8_t lock();

#endif
