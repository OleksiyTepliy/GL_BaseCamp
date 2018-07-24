#include "io.h"
#include <stdint.h>


/* some magic constants for Linear congruential generator */
/* for more https://en.wikipedia.org/wiki/Linear_congruential_generator */
struct random_param{
	uint16_t multiplier;
	uint16_t increment;
	uint16_t modulus;
	uint16_t seed;
};

struct random_param rv = {13, 2707, 65535, 32533};

// struct random_param rv = {13, 2707, 65535, 32533};

enum pwm {
	DURATION = 600U,	// duration
	FREQ = 100U	// pwm frequency
};


uint16_t random(uint16_t x)
{
	return (rv.multiplier * x + rv.increment) % (rv.modulus);
}


void soft_delay(volatile uint16_t N)
{
	/* If volatile is not used, AVR-GCC will optimize this stuff out     */
        /* making our function completely empty */
	volatile uint8_t inner = 0xFF;
	while (N--) {
		while (inner--);
	}
}


void pwm_generator(uint8_t lvl, enum pwm DURATION, enum pwm FREQ)
{
	if (lvl < 20) {
		DURATION /= 2;
	}
	uint16_t pwm_period = DURATION / FREQ;
	while (FREQ--) {
		/* one period of pwm */
		PORTB &= ~ (1 << 3);	// turn off led
		soft_delay(pwm_period - pwm_period * lvl / 100);	// wait 
		PORTB |= 1 << 3;		// turn on led
		soft_delay(pwm_period * lvl / 100);		// wait
	}
}


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
	/* Configure GPIO */
	DDRB |= 1 << 3;			/* set PB3 to output */
	PORTB |= 1 << 3;		/* set output to 1   */
	uint16_t rand = (rv.seed);
	uint8_t pwm = 5;
	while(1) {
		rand = random(rand);
		pwm = algorithm(rand);
		pwm_generator(pwm, DURATION, FREQ);
	}
	/* we should never get here */
	return 0;
}
