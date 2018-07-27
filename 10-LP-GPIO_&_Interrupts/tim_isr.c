#ifndef F_CPU
#define F_CPU   16000000UL
#endif

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

volatile uint8_t lvl = 0;	// global variable, sets pwm level
volatile _Bool flag = 0;


/* some magic constants for Linear congruential generator */
/* for more https://en.wikipedia.org/wiki/Linear_congruential_generator */
struct random_param{
	uint16_t multiplier;
	uint16_t increment;
	uint16_t modulus;
	uint16_t seed;
};

/* struct for custom parameters */
struct random_param rv = {13, 2707, 65535, 32533};



/**
 * Linear_congruential_generator 
 * @x:	takes previous number as a seed for next
 * 
 */
uint16_t random(uint16_t x)
{
	return (rv.multiplier * x + rv.increment) % (rv.modulus);
}


/**
 * Probability distribution  
 * function takes a random number and return pwm lvl
 * @r:	random number from generator
 * 
 */
uint8_t algorithm(uint16_t r)
{	// probability   25 20 13 8 7 6 7 6 5 1 1 1 %	min 0 max 65535
	if (r > 49151) {
    		return 100;  // maximum pwm level
	} else if (r > 36044) {
		return 92;
	} else if (r > 27525) {
		return 83;
	} else if (r > 22282) {
		return 75;
	} else if (r > 17694) {
		return 67;
	} else if (r > 13762) {
		return 59;
	} else if (r > 9174) {
		return 50;
	} else if (r > 5242) {
		return 42;
	} else if (r > 1966) {
		return 34;
	} else if (r > 1311) {
		return 25;
	} else if (r > 655) {
		return 17;
	} else 
		return 0;	// min pwm level
}


int main(void)
{
    	
	// set pin 6 as an output
	DDRD |= (1 << DDD6);
	// set PWM to 0
    	OCR0A = 0x00;
	// set none-inverting mode
    	TCCR0A |= (1 << COM0A1);
	// set fast PWM Mode
    	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	// set prescaler to 64, 1khz PWM
    	TCCR0B |= (1 << CS01) | (1 << CS00);


	// 20ms period, set top to decimal 155	16Mhz / 2 * Presc * (1 + OCRnA)
	OCR1A = 1200;	// 5ms
	// Mode 4 CTC, on OCR1A as a TOP level, timer overflows, interrupt generated.	
    	TCCR1B |= (1 << WGM12);
	// timer/counter 1 output compare A match interrupt is enabled.
    	TIMSK1 |= (1 << OCIE1A);
    	// set prescaler to 1024, and enable timer
	TCCR1B |= (1 << CS12) | (1 << CS10);

	uint16_t rand = (rv.seed);

    // enable interrupts
	sei();

	while (1)
	{
		rand = random(rand);
		lvl = algorithm(rand);	// update pwm level
		flag = 1;
        	while (flag);
	}
}


/** Timer1 Interrupt on overflow
 * We will change pwm period in this routine.
 * 
 */
ISR (TIMER1_COMPA_vect)
{
	//TCNT0 = 0x00; // reset Timer0 counter register
	OCR0A = (lvl * 255) / 100;	// set new pwm level
    	flag = 0;
}