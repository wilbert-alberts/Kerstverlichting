/*
 * main.c
 *
 *  Created on: 22 mrt. 2018
 *      Author: wilbert
 */

#define MINDIF (2) // ADC should differ at least

#include <avr/io.h>
#include <avr/interrupt.h>


void setupPWM() {
	// Attempting to generate pwm with
	// 50% dutycycle and 1Khz frequency.
	// By using toggle operation on output
	// pins two timer periods are needed
	// for one dutycycle period.
	// Timer needs to count to '500' for
	// one PWM period. So within 1ms
	// timer should tick 500 times.
	// Prescaler: 16, 500kHz
	// One ms is 500 ticks

	// Set OC1A and OC1B as output pins.
	DDRB = DDRB | (1<<DDB1) | (1<< DDB4);

	// Setup PWM
	// - Count to 250
	TCCR1 = TCCR1 | (1 << CTC1);
	OCR1C = 250;

	// - Ensure OC1A toggles on compare
	TCCR1 = TCCR1 & ~(1 << COM1A1) | (1 << COM1A0);

	// - Set compare value for OCR1A
	OCR1A = 10;

	// - Ensure OC1B toggles on compare
	//GTCCR = GTCCR & ~(1 << COM1B1) | (1 << COM1B0);

	// - Set compare value for OCR1B
	OCR1B = 0;

	// Enable interrupt on OCR1B compare
	TIMSK = TIMSK | (1<<OCIE1B);

	// Start timer, prescaler 16
	TCCR1 = TCCR1 & 0xf0 | 0x02;
}

// This program reads the input of ADC1 and generates
// two 50% PWM signals that are set out of phase by ADC1

// HSI
// adc:  ADC1, input,  pin 7
//       AREF, input,  pin 5
// pwm1: OC1B, output, pin 3
// pwm2: OC1A, output, pin 6



static void setupADC()
{
	// Setup ADC
	// - Set AREF
	ADMUX = ADMUX & ~(1<< REFS1) | (1<<REFS0);
	// - Select ADC1
	ADMUX = ADMUX & 0xf0 | 0x01;
	// - ADC clock to 125Khz, prescaler 64
	ADCSRA = ADCSRA & ~(1<<ADPS0) | (1<<ADPS1);
	// - Disable digital input on ADC1
	DIDR0 = DIDR0 | (1<<ADC1D);
	// - Turn ADC on
	ADCSRA = ADCSRA | (1<<ADEN);
}

static void setPWM(uint16_t phase)
{
	// phase is to be interpreted as a
	// 10 bit number. This is to be translated
	// into an offset for the two
	// pwm signals.

	// Due to the analogue circuit, the max
	// output shall never exceed 3.3 * 10/13
	// = 2.53V

	float dc = phase/790.0;
	dc = dc*250;
	if (dc>=250)
		dc = 249;
	uint8_t newcompare = (uint8_t)dc;
	uint8_t oldcompare = OCR1B;

	if (oldcompare>newcompare && oldcompare-newcompare>MINDIF)
		OCR1B = newcompare;
	if (newcompare>oldcompare && newcompare-oldcompare>MINDIF)
		OCR1B = newcompare;
}


static void getADC(uint16_t* adc)
{
	// Start ADC
	ADCSRA = ADCSRA | (1<<ADSC);

	// While not ready, wait
	while (ADCSRA & (1<<ADSC));

	// Capture result
	*adc=ADC;
}

int main(void)
{
	uint16_t adcIn = 0;

	setupPWM();
	setupADC();

	sei();
	while(1) {
		setPWM(adcIn);
		getADC(&adcIn);
	}
}

ISR(TIM1_COMPB_vect)
{
//	static uint8_t even = 1;

//	even= 1-even;

//	if (even==0) {
		// Force
		uint8_t v = PINB & (1<<PINB1);
		if (v) {
			PORTB = PORTB |  (1<<PORTB4);
		}
		else {
			PORTB = PORTB & ~(1<<PORTB4);

		}
//	}
}

