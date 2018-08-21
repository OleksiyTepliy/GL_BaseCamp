#include "onewire.h"
#include <avr/io.h>
#include "uart.h"
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#define KEY_SIZE 8

enum state {
	LOCKED = 0,
	UNLOCKED = 1,
	DEFAULT = 0,
	KEY_SET,
	SET_NEW
};

enum state lock_state;
enum state key_state;

// eeprom key storage
uint8_t EEMEM EE_DEF_KEY[KEY_SIZE];
// ram storage
uint8_t RAM_DEF_KEY[KEY_SIZE] = {0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24};

void soft_delay_us(uint16_t time)
{
	while (time--) {
		_delay_us(1.0);
	}
}

void line_low(void)	// PB0 - arduino pin8 is 1wire data pin
{
	PORTB &= ~(1 << 0);	// reset pin
	DDRB |= (1 << 0);	// set to output
}

void line_release(void)
{
	DDRB &= ~(1 << 0);	// set to input
	PORTB |= (1 << 0);	// resistor enable
}

bool line_read(void)
{
	uint8_t val = PINB & 0x01;	// read PB0
	return val;
}

void clear_buff()
{
	rdbuff[0] = '\0';
}

void _lock()
{
	TCCR1B |= (1 << CS12);
	OCR1B = 40; // min angle, locked
	_delay_ms(600);
	TCCR1B &= ~(1 << CS12);
}

void _unlock()
{
	TCCR1B |= (1 << CS12);
	OCR1B = 160; // max angle, unlocked
	_delay_ms(600);
	TCCR1B &= ~(1 << CS12);
}


bool key_compare(uint8_t compared_key[])
{
	bool key_match = false;
	uint8_t base_key[KEY_SIZE];
	int8_t j;
	eeprom_read_block((void*) &base_key[0], (const void*) &EE_DEF_KEY[0], KEY_SIZE);
	j = 0;	// last index of key array
	while (j < KEY_SIZE && compared_key[j] == base_key[j]) {
			j++;
	}
	return key_match = (j == KEY_SIZE) ? true : false;
}


int main()
{
	/* We use internal pullup resitor for 1-wire line */
	DDRB = (1 << 1) | (1 << 3);	// output pin9, pin11
	PORTB |= (1 << 0);	// 1wire pin8
	DDRD = (1 << 6) | (1 << 7); // PD6 PD7 LED pins
	PORTD &= (~(1 << 6)); // turn off green led
	PORTD &= (~(1 << 7)); // turn off red led

				/*** SERVO TIMER INIT BEGIN***/

	TCCR1A = 0; // reset Timer Control Register A
        TCCR1B = 0; // reset Timer Control Register B

	DDRB |= (1 << DDB2); // output pin10

	/* Clear OC1A/OC1B on compare match, set OC1A/OC1B at
	BOTTOM (Non-inverting mode) */
	TCCR1A |= (1 << COM1A1)|(1 << COM1B1);

	/* SET MODE 14 Fast PWM ICR1 BOTTOM */
	// WGM1[3] = 1 WGM1[2] = 1 WGM1[1] = 1 ICR1 = TOP
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);

	ICR1 = 1250 - 1; // Set TOP to 40000
	OCR1B = 40;

	// start timer, prescaler 256, close lock
	TCCR1B |= (1 << CS12);
	_delay_ms(600);
	TCCR1B &= ~(1 << CS12);
				/*** SERVO TIMER INIT END ***/


				/*** UART INIT BEGIN***/
	uart_init();
	sei();
	uart_put("\n\nHello from avr\n\n");
				/*** UART INIT END***/	

	ow_Pin pin;
	ow_Pin_init(&pin, &line_low, &line_release, &line_read, &soft_delay_us, 1, 15, 60, 2);
	ow_err err;

	uint8_t ibutton_id[KEY_SIZE];
	uint8_t crc;

	char *lock[] = {"\nLOCKED\n", "\nUNLOCKED\n"};
	char *key[] = {"\nDEFAULT\n", "\nKEY SET\n", "\nCOPY NEW KEY\n"};
	_lock();
	lock_state = LOCKED;
	key_state = key_compare(RAM_DEF_KEY) ? DEFAULT : KEY_SET;

	while (1)
	{
				/*** UART COMMANDS BEGIN ***/

		if (atomic_str_eq((char *) rdbuff, "show key")) {
			uint8_t var_key[KEY_SIZE + 1]; // for proper string output
			eeprom_read_block((void*) &var_key[0], (const void*) &EE_DEF_KEY[0], KEY_SIZE);
			var_key[KEY_SIZE] = '\0';
			uart_put((char *) var_key);
			clear_buff();
		} else if (atomic_str_eq((char *) rdbuff, "key state")) {
			uart_put(key[key_state]);
			clear_buff();
		} else if (atomic_str_eq((char *) rdbuff, "set key")) {
			key_state = SET_NEW;
			uart_put(key[key_state]);
			clear_buff();
		} else if (atomic_str_eq((char *) rdbuff, "reset key")) {
			key_state = DEFAULT;
			eeprom_write_block((void*) &RAM_DEF_KEY[0], (void*) &EE_DEF_KEY[0], KEY_SIZE);
			uart_put(key[key_state]);
			clear_buff();
		} else if (atomic_str_eq((char *) rdbuff, "lock")) {
			if (lock_state == UNLOCKED) {
				_lock();
				lock_state = LOCKED;
			}
			else {
				uart_put(lock[lock_state]);
			}
			clear_buff();
		} else if (atomic_str_eq((char *) rdbuff, "unlock")) {
			if (lock_state == LOCKED) {
				_unlock();
				lock_state = UNLOCKED;
			}
			else {
				uart_put(lock[lock_state]);
			}
			clear_buff();
		} else if (atomic_str_eq((char *) rdbuff, "state")) {
			uart_put(lock[lock_state]);
			clear_buff();
		} else {
			uart_put("Unknown command\n");
			uart_put(rdbuff);
			clear_buff();
		}
				
				/*** UART COMMANDS END ***/

		err = ow_cmd_readrom(&pin, ibutton_id, &crc, true, false);

		if (err == OW_EOK) {
			if (key_state == SET_NEW) {
				eeprom_write_block((void*) &ibutton_id[0], (const void*) &EE_DEF_KEY[0], KEY_SIZE);
				key_state = KEY_SET;
				uart_put(key[key_state]);
				PORTD |= (1 << 6); // pin6 GREEN ON. KEY INJECTED IN EEPROM
				_delay_ms(2000);
				PORTD &= ~(1 << 6); // PD6 GREEN OFF.
			} else {
				bool result = key_compare(ibutton_id);
				/* if keys are the same */
				if (result) {
					PORTD |= (1 << 6); // pin6 GREEN ON. CRC OK. KEY VALID.
					_delay_ms(400);
					PORTD &= ~(1 << 6); // pin6 GREEN OFF.
					if (lock_state == LOCKED) {
						_unlock();
						lock_state = UNLOCKED;
					}
				}
				else {
					PORTD |= (1 << 7); // pin7 RED ON. CRC WRONG, KEY NOT VALID.
					_delay_ms(400);
					PORTD &= ~(1 << 7);
				}
			}
		}
	}
}


