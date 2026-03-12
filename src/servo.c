#include <avr/io.h>
#include <avr/interrupt.h>
#include "servo.h"

/*
 * HOW RC SERVO CONTROL WORKS
 * ---------------------------
 * RC servos expect a pulse sent repeatedly at ~50 Hz (every 20 ms).
 * The WIDTH of each pulse tells the servo what angle to move to:
 *
 *   Pulse width  |  Position
 *   -------------|----------
 *    ~1.0 ms     |  -100%  (full left / min angle)
 *    ~1.5 ms     |     0%  (center)
 *    ~2.0 ms     |  +100%  (full right / max angle)
 *
 * This code uses Timer1 to generate that timing via three interrupts:
 *
 *   1. OVF  (overflow)  fires every ~20 ms → drives both pins HIGH (pulse starts)
 *   2. COMPA            fires when TCNT1 == OCR1A → drives pin 1 LOW  (pulse ends)
 *   3. COMPB            fires when TCNT1 == OCR1B → drives pin 2 LOW  (pulse ends)
 *
 * The gap between OVF and COMPA/COMPB is the pulse width, and that width
 * is what determines the servo angle.
 *
 * TIMER TICK RATE
 * ---------------
 * At 16 MHz with a /8 prescaler, the timer increments every 0.5 µs.
 * So 1 ms = 2000 ticks, 1.5 ms = 3000 ticks, 20 ms = 40000 ticks.
 */

/* Number of timer ticks in one 20 ms servo period (40000 × 0.5 µs = 20 ms). */
#define TIME_VALUE	(40000)

/* The timer is 16-bit (max 65535). To make it overflow after exactly TIME_VALUE
 * ticks, we preload it to (65536 - TIME_VALUE) each cycle instead of starting
 * from 0. The small offset in 65636 vs 65536 is a hardware-tuned correction. */
#define RESET_VALUE	(65636UL - TIME_VALUE)

/* Pulse width for the center position (0%).
 * TIME_VALUE * 0.075 = 40000 * 0.075 = 3000 ticks = 3000 × 0.5 µs = 1.5 ms. */
#define STOP_VALUE	(TIME_VALUE * 0.075) // (TIME_VALUE * 0.076)

/* Half the total travel range in ticks.
 * TIME_VALUE * 0.03 = 1200 ticks = 600 µs.
 * At +100% the pulse is 1.5 ms + 0.6 ms = 2.1 ms (full right).
 * At -100% the pulse is 1.5 ms - 0.6 ms = 0.9 ms (full left).
 * Increase this constant to widen the travel range; decrease to narrow it. */
#define RANGE		(TIME_VALUE * 0.03)

/* Fires every ~20 ms when the timer overflows.
 * Reloads the timer and drives both servo pins HIGH to start a new pulse. */
ISR(TIMER1_OVF_vect) {
	TCNT1 = RESET_VALUE;   // Reload so the next overflow is exactly TIME_VALUE ticks away.

	BITSET(PORT_1, PIN_1); // Pulse starts: pin 1 goes HIGH.
}

/* Fires when TCNT1 reaches OCR1A (set by servo1_set_percentage).
 * The time elapsed since the OVF interrupt equals the pulse width for servo 1.
 * Driving the pin LOW here ends that pulse and locks in the servo angle. */
ISR(TIMER1_COMPA_vect) {
	BITCLR(PORT_1, PIN_1); // Pulse ends: pin 1 goes LOW.
}


void init_servo(void) {
	// Config pins as output
	BITSET(DDR_1, PIN_1);


	// Use mode 0 (normal, no hardware PWM), clkdiv = 8 → 0.5 µs per tick at 16 MHz.
	TCCR1A = 0;
	TCCR1B = (0 << CS12) | BIT(CS11) | (0 << CS10);
	// Enable interrupts: output-compare A, output-compare B, and overflow.
	TIMSK1 = BIT(OCIE1A) | BIT(TOIE1);

	TCNT1 = RESET_VALUE;

}

/*
 * Set the angle of servo 1.
 *
 * percentage: -100 (full left) to +100 (full right), 0 = center.
 *
 * HOW POSITION IS SET
 * -------------------
 * OCR1A is the compare value that triggers the COMPA interrupt (pin goes LOW).
 * The timer starts at RESET_VALUE each period, so the pulse width in ticks is:
 *
 *   OCR1A - RESET_VALUE  =  STOP_VALUE + (RANGE / 100 * percentage)
 *                        =  3000       + (±1928)
 *
 * At   0%: 3000 ticks = 1.5 ms → center
 * At +100%: 4200 ticks = 2.1 ms → full right
 * At -100%: 1800 ticks = 0.9 ms → full left
 *
 * HOW TO CONTROL DISTANCE (ANGLE)
 * --------------------------------
 * Pass a different percentage value. The servo will jump immediately to
 * that angle the next time the OVF interrupt fires (~20 ms away at most).
 *
 * HOW TO CONTROL SPEED
 * --------------------
 * This function sets a target angle instantly — the servo moves as fast
 * as its motor allows. To make it appear to move slowly, call this
 * function repeatedly with small incremental steps and a delay between
 * each step (see main.c for an example pattern using large jumps).
 * Smaller steps + shorter delays = smoother, slower-looking motion.
 */
void servo1_set_percentage(signed char percentage) {
	if (percentage >= -100 && percentage <= 100) {
		OCR1A = RESET_VALUE + STOP_VALUE + (RANGE / 100 * percentage);
	}
}

