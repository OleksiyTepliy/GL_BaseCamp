#ifndef F_CPU
#define F_CPU   16000000UL
#endif

#include "segm.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

struct clock {
	volatile uint8_t hh;
	volatile uint8_t min;
	volatile uint8_t sec;
};

struct clock time = {8, 00, 00};
struct clock alarm = {7, 0, 0};

const enum state {
	HH_MM_MODE = 0,
	MM_SS_MODE,
	SET_TIME_MODE,
	SET_ALARM_MODE,
	BEEP
};

volatile enum state STATE = HH_MM_MODE;	// global state flag
volatile uint16_t tick = 0; // tick counts seconds.
volatile uint16_t get_tick = 0;	// get_tick counts miliseconds used to set debounce time.
volatile uint8_t curr_state_PD2, curr_state_PD3, curr_state_PD7;
volatile uint16_t last_tick_PD2 = 0, last_tick_PD3 = 0, last_tick_PD7 = 0;
uint16_t bounce_time = 170;
int t_delay = 200, a_delay = 200; //delay for displaying current mode
uint8_t dot = 0x00; // dot segment blink every second

/**
 * Enables sleep mode to reduce power consumption.
 * ms_val :	sleep time in miliseconds.
 */  
void sleep_ms(uint16_t ms_val)
{
	set_sleep_mode(SLEEP_MODE_IDLE);	// Idle Mode, allows timer 1 to work
	cli();		/* Disable interrupts -- as memory barrier */
	sleep_enable();	/* Set SE (sleep enable bit) */
	sei();  	/* Enable interrupts. We want to wake up, don't we? */
	while (ms_val--) {
		sleep_cpu();	/* Put MCU to sleep */
		/* This is executed after wakeup */
		/* We Will Wake Up In ISR(TIMER1_COMPA_vect, ISR_BLOCK) */
	}
	sleep_disable();	/* Disable sleeps for safety */		
}


static struct segm_Port PB = {
	.DDR = &DDRB,
	.PIN = &PINB,
	.PORT = &PORTB,
};


static struct segm_Display display = {
	.SHCP = {.port = &PB, .pin = 0},
	.STCP = {.port = &PB, .pin = 1},
	.DS   = {.port = &PB, .pin = 2},
	.delay_func = &_delay_loop_1,	/* 3 cycles / loop, busy wait */
	.sleep_ms_func = &sleep_ms,	/* 3 cycles / loop, busy wait */
	.is_comm_anode = false		/* We have common cathode display */
};


void fill(uint8_t symbols[], struct clock time_str, enum state mode)
{
	uint8_t tmp;
	switch (mode) {
		case MM_SS_MODE :
			tmp = time_str.sec % 10;
			symbols[3] = segm_sym_table[tmp];
			tmp = time_str.sec / 10;
			symbols[2] = segm_sym_table[tmp];
			tmp = time_str.min % 10;
			symbols[1] = segm_sym_table[tmp] + dot; // add dot
			tmp = time_str.min / 10;
			symbols[0] = segm_sym_table[tmp];
			break;
		default :
			tmp = time_str.min % 10;
			symbols[3] = segm_sym_table[tmp];
			tmp = time_str.min / 10;
			symbols[2] = segm_sym_table[tmp];
			tmp = time_str.hh % 10;
			symbols[1] = segm_sym_table[tmp] + dot; // add dot
			tmp = time_str.hh / 10;
			symbols[0] = segm_sym_table[tmp];
	}
}


void fill_mode_name(uint8_t symbols[], enum state mode)
{
	symbols[0] = segm_sym_table[5];	//S
	symbols[1] = 0x79; //E
	symbols[2] = 0x78 + segm_sym_DP; // t.
	if (mode == SET_TIME_MODE) {
		symbols[3] = 0x78; // t
	} else if (mode == SET_ALARM_MODE) {
		symbols[3] = 0x77; // A
	}
}


void fill_symbols(uint8_t symbols[])
{
	if (time.sec >= 60) {
		(time.min)++;
		time.sec %= 60;
	}
	if (time.min >= 60) {
		(time.hh)++;
		time.min %= 60;
	}
	if (time.hh >= 24) {
		time.hh %= 24;
	}

	switch (STATE) {
	case MM_SS_MODE :
		fill(symbols, time, STATE);
		break;
	case SET_TIME_MODE :
		// make some delay, displays current mode.
		while (t_delay > 0) {		
			fill_mode_name(symbols, STATE);
			--t_delay;
			break;
		}
		if (t_delay == 0) {
			fill(symbols, time, STATE);
		}
		a_delay = 100;
		break;
	case SET_ALARM_MODE :
		// make some delay, displays current mode.
		while (a_delay > 0) {
			fill_mode_name(symbols, STATE);
			--a_delay;
			break;
		}
		if (a_delay == 0) {
			fill(symbols, alarm, STATE);
		}
		t_delay = 100;
		break;
	default :
		fill(symbols, time, HH_MM_MODE);
	}
}


			/* MAIN BEGIN */

int main(void)
{
			/* START INIT */
	/* Init Timer1  in Clear Timer on Compare Match Mode for clocking */
TCCR1B |= (1 << WGM12);	// mode 4 CTC Mode
OCR1A |= 2000 - 1; //fOCnA == 1khz; generate interrupt every milisecond.
TIMSK1 |= (1 << OCIE1A); // enable timer interrupt bit

	/* Init Timer0 in Clear Timer on Compare Match Mode for Buzzer */
TCCR0A |= (1 << COM0A0); // Toggle OC0A on compare match.
TCCR0A |= (1 << WGM01); // according to datasheet need set WGM0[1] = 1 WGM0[0] = 0 WGM0[2] = 0	
OCR0A |= 80; // 80 gives better sound quality.

			/* Init GPIO */
DDRD |= (1 << DDD6); // set output pin for Buzzer Alarm
DDRB |= (1 << 5) | (1 << DDB3);	// pin 13 onboard LED, pin 11 ALARM LED.
PORTB &= (~(1 << DDB3)); // set ALARM LED to logic 0.

			/* Init Buttons */
DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD7); // set pin 2, 3, 7 as an options button input.
PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD7); // enable pull up resistor
PCMSK2 |= (1 << PCINT23); // interrutp enable, Pin Change Enable Mask.
PCICR  |= (1 << PCIE2); // The PCIEx bits in the PCICR registers enable 
// PD2 and PD3, external interrupts enable.
// ISCx11 ISCx00 The falling edge of INTx generates an interrupt request 
EICRA |= (1 << ISC01) | (1 << ISC11);	// set trigger on falling edge
EIMSK |= (1 << INT0) | (1 << INT1); // interrupt mask, enable isr.

segm_init(&display);

uint8_t symbols[4];	//array of 7segment numbers
sei(); // enable interrupts
TCCR1B |= (1 << CS11); // set prescaler to 256 and start timer
			/* END INIT */

	while(1) {
		fill_symbols(symbols);
		segm_indicate4(&display, symbols);
	}
}
			
			/* MAIN END */

// Timer0 interrupt routine.
ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
	++get_tick;
	++tick;
	if (tick >= 1000 ) {
		tick %= 1000;
		++(time.sec);
		dot = (dot == 0x00) ? segm_sym_DP : 0x00; // blink dot every second.
		PORTB ^= (1 << PB5); // toggle onboard LED
		if (STATE == BEEP) {
			// TOGGLE ALARM LED
			PORTB ^= (1 << PB3);
			// START TIMER0, BUZZER ON
			TCCR0B ^= (1 << CS10) | (1 << CS11);
			//OCR0A += 10;
		}
	}
	if (time.min == alarm.min && time.hh == alarm.hh 
	    && time.sec == alarm.sec) {
		STATE = BEEP;
	}
}

// external interrupt routine, button PD2.
ISR (INT0_vect)
{
	curr_state_PD2 = (PIND & (1 << PD2));	// check button state
	if (curr_state_PD2 == 0 && get_tick - last_tick_PD2 > bounce_time) {
		if (STATE == SET_TIME_MODE) {
			time.hh = ++(time.hh) % 24;
		}
		if (STATE == SET_ALARM_MODE) {
			alarm.hh = ++(alarm.hh) % 24;
		}
	}
	last_tick_PD2 = get_tick;
}

// external interrupt routine, button PD3.
ISR (INT1_vect)
{
	curr_state_PD3 = (PIND & (1 << PD3));	// check button state
	if (curr_state_PD3 == 0 && get_tick - last_tick_PD3 > bounce_time) {
		if (STATE == SET_TIME_MODE) {
			time.min = ++(time.min) % 60;
		}
		if (STATE == SET_ALARM_MODE) {
			alarm.min = ++(alarm.min) % 60;
		}
	}
	last_tick_PD3 = get_tick;
}


// pin change interrupt routine, button PD7.
ISR (PCINT2_vect)
{
	curr_state_PD7 = (PIND & (1 << PD7));
	if (curr_state_PD7 == 0 && get_tick - last_tick_PD7 > bounce_time) {
		switch (STATE) {
		case HH_MM_MODE : 
			STATE = MM_SS_MODE; break;
		case MM_SS_MODE : 
			STATE = SET_TIME_MODE; break;
		case SET_TIME_MODE :
			STATE = SET_ALARM_MODE; break;
		case BEEP :
			// TURN OFF ALARM LED
			PORTB &= (~(1 << PB3));
				// Stop Timer0, BUZZER OFF
			TCCR0B |= (~(1 << CS10)) | (~(1 << CS11));
		default :
			STATE = HH_MM_MODE;
		}
	}
	last_tick_PD7 = get_tick;
}