/*
 * main.c
 *
 *  Created on: 22 mrt. 2018
 *      Author: wilbert
 * This program reads the input of ADC1 and generates
 * two 50% PWM signals that are set out of phase by ADC1

 * HSI
 * adc:  ADC1, input,  pin 7
 *       AREF, input,  pin 5
 * pwm1: OC1B, output, pin 3
 * pwm2: OC0B, output, pin 6

 */

#define MINDIF (2) // ADC should differ at least

#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t pwm = 0;

void setupPWM() {

	// Set OC0B (PB1)as output pins.
	DDRB = DDRB | (1 << DDB1);

	// Set OC1B (PB4) as output pins.
	DDRB = DDRB | (1 << DDB4);

	// Start timer 1, clock: 8Mhz, prescaler 32
	// Timer 1 will be running with frequency of 256 Hz
	TCCR1 = (TCCR1 & 0xf0) | (1<<CS13);

}

static void setupADC() {
	// Setup ADC
	// - Set AREF
	ADMUX = (ADMUX & ~(1 << REFS1)) | (1 << REFS0);
	// - Select ADC1
	ADMUX = (ADMUX & 0xf0) | 0x01;
	// - ADC clock to 125Khz, prescaler 64
	ADCSRA = (ADCSRA & ~(1 << ADPS0)) | (1 << ADPS1) | (1 << ADPS2);
	// - Disable digital input on ADC1
	DIDR0 = DIDR0 | (1 << ADC1D);
	// - Turn ADC on
	ADCSRA = ADCSRA | (1 << ADEN);
}


int main(void) {
	volatile uint8_t cnt;
	volatile uint8_t scnt;
	setupPWM();
	setupADC();

	// Start ADC
	ADCSRA = ADCSRA | (1 << ADSC);

	sei();
	while (1) {
		if (~(ADCSRA & (1 << ADSC))) {
			// ADC finished
			pwm = ADC/8;
			// Start ADC
			ADCSRA = ADCSRA | (1 << ADSC);
		}

		cnt = TCNT1;
		scnt = cnt + pwm;
		if (cnt<128) {
			PORTB = PORTB & ~(1<<PORTB1);
		}
		else {
			PORTB = PORTB | (1<<PORTB1);

		}
		if (scnt<128) {
			PORTB = PORTB & ~(1<<PORTB4);
		}
		else {
			PORTB = PORTB | (1<<PORTB4);

		}

	}
}


