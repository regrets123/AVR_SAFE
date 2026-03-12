#ifndef _SERVO_H_
#define _SERVO_H_

/* Set a single bit in a register.
 * BITSET(PORTD, PD5)  =>  PORTD |= (1 << PD5)  */
#define BITSET(reg, bit)  ((reg) |= (1 << (bit)))

/* Clear a single bit in a register.
 * BITCLR(PORTD, PD5)  =>  PORTD &= ~(1 << PD5)  */
#define BITCLR(reg, bit)  ((reg) &= ~(1 << (bit)))

/* Create a bitmask with only bit n set.
 * BIT(CS11)  =>  (1 << CS11)  */
#define BIT(n)            (1 << (n))

#define PORT_1	PORTD
#define PIN_1	PD5
#define DDR_1	DDRD

void init_servo(void);
void servo1_set_percentage(signed char percentage);
 
#endif /* _SERVO_H_ */
