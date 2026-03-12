#ifndef SAFE_H_
#define SAFE_H_

#include <stdint.h>

typedef enum { locked, unlocked, firstInput, secondInput, thirdInput, lastInput, idle } SafeState;

/* Bitmask tracking which analog inputs are unlocked.
 * Bit 0 = A0, bit 1 = A1, bit 2 = A2. A set bit means unlocked. */
extern uint8_t inputLockState;


uint8_t lightLed(uint8_t index);
uint8_t update();
uint8_t unlock();
uint8_t lock();


#endif
