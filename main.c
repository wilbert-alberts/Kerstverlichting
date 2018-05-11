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

static uint16_t adcIn = 0;
static uint8_t pwm = 0;

void setupPWM() {
	// Attempting to generate pwm with
	// 50% dutycycle and 1Khz frequency.
	// By using toggle operation on output
	// pins two timer periods are needed
	// for one dutycycle period.
	// Prescaler set to 1, cpu running on 8Mhz
	// Counting to 250 lasts 250 us
	// So one period is 500 us.
	// PWM frequency 2 Khz.

	// Set OC0B (PB1)as output pins.
	DDRB = DDRB | (1 << DDB1);

	// - Ensure OC0B toggles on compare
	TCCR0A = TCCR0A & ~((1 << COM0B1) | (1 << COM0B0));
	TCCR0A = TCCR0A | (1 << COM0B0);

	// - Ensure clear on timer compare
	TCCR0A = TCCR0A & ~((1 << WGM01) | (1 << WGM00));
	TCCR0B = TCCR0B & ~(1 << WGM02);

	TCCR0A = TCCR0A | (1 << WGM01);

	// - Count to 250
	OCR0B = 250;

	// Enable interrupt on OCR0B compare
	TIMSK = TIMSK | (1 << OCIE0B);

	// Set OC1B (PB4) as output pins.
	DDRB = DDRB | (1 << DDB4);

	// - Ensure OC1B toggles on compare
	GTCCR = GTCCR & ~((1 << COM1B1) | (1 << COM1B0));
	GTCCR = GTCCR | (1 << COM1B0);

	// - Ensure clear on timer compare
	GTCCR = GTCCR | (1 << CTC1);

	// - Count to 250
	OCR1B = 250;
	OCR1C = 250;

	// Enable interrupt on OCR0B compare
	TIMSK = TIMSK | (1 << OCIE1B);

	// Start timer 0, clock: 8 Mhz, prescaler 1
	TCCR0B = (TCCR1 & 0xf8) | 0x01;

	// Start timer 1, clock: 8Mhz, prescaler 1
	TCCR1 = (TCCR1 & 0xf0) | 0x01;
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

static void getADC()
{
	float f;

	// Start ADC
	ADCSRA = ADCSRA | (1 << ADSC);

	// While not ready, wait
	while (ADCSRA & (1 << ADSC))
		;

	// Capture result
	adcIn = ADC;

	// Determine PWM
	f = adcIn / 1024.0;
	pwm = 250 * f;
}

int main(void) {
	setupPWM();
	setupADC();

	sei();
	while (1) {
		getADC();
	}
}

ISR(TIM1_COMPA_vect) {
}

ISR(TIM1_COMPB_vect) {
	TCNT0 = pwm;
}

