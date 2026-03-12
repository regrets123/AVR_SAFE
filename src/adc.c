#include <avr/io.h>
#include "adc.h"

/*
 * HOW THE ADC WORKS
 * -----------------
 * The ATmega328P has a 10-bit successive-approximation ADC.
 * It converts an analog voltage (0–Vref) to a 10-bit integer (0–1023).
 *
 * The ADC clock must be between 50 kHz and 200 kHz for full 10-bit accuracy.
 * At 16 MHz with a /128 prescaler: 16 000 000 / 128 = 125 kHz. ✓
 *
 * ADMUX register:
 *   REFS1:REFS0 = 01  → AVcc (5 V) as voltage reference
 *   MUX3:MUX0        → selects the input channel (0 = ADC0 = A0)
 *
 * ADCSRA register:
 *   ADEN  → enables the ADC
 *   ADSC  → starts a single conversion; hardware clears it when done
 *   ADPS2:ADPS0 = 111 → clock prescaler /128
 */

void adc_init(void) {
    /* AVcc reference, channel 0 selected by default */
    ADMUX  = (1 << REFS0);
    /* Enable ADC, prescaler /128 → 125 kHz ADC clock at 16 MHz */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/*
 * Read the specified ADC channel (0–7 for A0–A7).
 * Returns a 10-bit value (0–1023) proportional to 0–Vcc.
 */
uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
