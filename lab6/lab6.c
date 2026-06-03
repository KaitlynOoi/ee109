#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "lcd.h"

void timer1_init(void);
void play_tone(uint16_t, uint16_t);

volatile uint8_t new_state, old_state;
volatile uint8_t changed;			// Flag that count changed (set in ISR)
volatile int16_t count = 20;	// Count to display (10..80)
volatile uint16_t buzz_count; // How many transitions the ISR has to do
volatile uint8_t buzzing;			// Flag that tone is being played

int main(void)
{
	uint8_t bits, a, b;
	uint8_t i;
	char ostr[20];
	uint16_t tone_length;
	// Frequencies for tones from 400Hz to 1600Hz in 50Hz steps (25 tones)
	uint16_t frequency[25];

	// Initialize appropriate DDR and PORT registers
	PORTC |= (1 << PC5) | (1 << PC1); // Enable pull-ups on PC5, PC1
	DDRB |= (1 << PB4);								// Set PORTB bit for buzzer output

	PORTD |= (1 << PD2); // Turn on pull-ups for button

	timer1_init(); // Initialize TIMER1 for tones

	lcd_init(); // Initialize the LCD

	// Add code here to write the splash screen
	lcd_moveto(0, 0);
	lcd_stringout("Kaitlyn Ooi");

	int month = 10;
	int day = 4;
	int year = 2005;
	char date[17];
	snprintf(date, sizeof(date), "%02d/%02d/%04d", month, day, year);
	int length = strlen(date);
	int start = (16 - length) / 2;
	if (start < 0)
		start = 0;
	lcd_moveto(1, start);
	lcd_stringout(date);

	// Add a 1 second delay so the splash screen can be seen
	_delay_ms(1000);
	// Add command to clear the screen after the delay
	lcd_writecommand(0x01);

	// Set up frequencies
	for (i = 0; i < 25; i++)
		frequency[i] = (uint16_t)(i * 50 + 400);

	// Determine the initial state (read both encoder pins once)
	bits = PINC;
	a = bits & (1 << PC1);
	b = bits & (1 << PC5);
	// State encoding: 0 = 00, 1 = 01, 2 = 10, 3 = 11 (b is MSB)
	if (!b && !a)
		old_state = 0;
	else if (!b && a)
		old_state = 1;
	else if (b && !a)
		old_state = 2;
	else
		old_state = 3;

	// new_state = old_state;

	PCICR |= (1 << PCIE1);										// Enable PCINT[14:8]
	PCMSK1 |= (1 << PCINT9) | (1 << PCINT13); // PC1=PCINT9, PC5=PCINT13

	sei(); // Enable global interrupts

	while (1) // Loop forever
	{
		// Check if count changed and update LCD if so
		if (changed)
		{
			changed = 0;

			if (count > 80)
				count = 80;
			else if (count < 10)
				count = 10;

			snprintf(ostr, 5, "%4d", count);
			lcd_moveto(1, 0);
			lcd_stringout(ostr);
		}
		// See if button was pressed to play the sequence
		if ((PIND & (1 << PD2)) == 0)
		{
			tone_length = (uint16_t)((uint32_t)count * 100UL / 25UL); // ms per tone
			for (i = 0; i < 25; i++)
				play_tone(frequency[i], tone_length);
		}
	}
	return 0;
}

/*
		play_tone - Plays a single tone on the buzzer at frequency "freq" and
		for "length" milliseconds
*/
void play_tone(uint16_t freq, uint16_t length)
{
	uint32_t temp;

	// Wait for previous tone to complete
	while (buzzing == 1)
	{
	}

	/*
			freq is the number of cycles/sec of the tone, so
			the number of transitions/sec would be freq * 2.
			Want to play the tone for "length" milliseconds.
			The number of transitions should be given by
	 freq * 2 * length / 1000
		*/
	temp = (uint32_t)freq;
	temp = temp * 2u * (uint32_t)length / 1000u;
	buzz_count = (uint16_t)temp;
	/*
	 Find counter modulo.  The number of system clocks
	 for half a period using a prescalar of 1 is
	 16,000,000 / (freq * 2) = 8000000/freq
 */

	TCNT1 = 0;
	OCR1A = 8000000 / freq - 1;
	TCCR1B |= (0b001 << CS10); // Set prescaler to /1, starts TIMER1
	buzzing = 1;							 // Flag that the buzzing is going on
}

void timer1_init()
{

	TCCR1A = 0x00;
	TCCR1B = (1 << WGM12);
	TIMSK1 = (1 << OCIE1A); // Enable Output Compare A Match Interrupt
}

/*
		ISR for TIMER1 Compare A: toggle buzzer pin, decrement buzz_count, stop when done
*/
ISR(TIMER1_COMPA_vect)
{
	if (buzz_count > 0)
	{
		// Toggle PB4
		PINB = (1 << PB4);

		buzz_count--;

		if (buzz_count == 0)
		{
			// Stop Timer1
			TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
			buzzing = 0; // indicate tone finished
		}
	}
	else
	{
		// ensure timer stopped
		TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
		buzzing = 0;
	}
}

/*
		ISR for Pin Change on PORTC (PCINT1_vect): handles encoder state machine.
		Reads PINC once (atomic), computes new state, updates count, clamps, signals main.
*/
ISR(PCINT1_vect)
{
	uint8_t bits = PINC;					 // read both encoder pins at once
	uint8_t a = bits & (1 << PC1); // LSB
	uint8_t b = bits & (1 << PC5); // MSB
	uint8_t newstate;

	// Determine new state from a and b
	if (!b && !a)
		newstate = 0;
	else if (!b && a)
		newstate = 1;
	else if (b && !a)
		newstate = 2;
	else
		newstate = 3;

	// Update count based on state transition
	if (newstate != old_state)
	{
		if (old_state == 0)
		{
			if (newstate == 1)
				count++;
			else if (newstate == 2)
				count--;
		}
		else if (old_state == 1)
		{
			if (newstate == 0)
				count--;
			else if (newstate == 3)
				count++;
		}
		else if (old_state == 2)
		{
			if (newstate == 3)
				count--;
			else if (newstate == 0)
				count++;
		}
		else
		{
			if (newstate == 2)
				count++;
			else if (newstate == 1)
				count--;
		}

		if (count > 80)
			count = 80;
		else if (count < 10)
			count = 10;
		// Update state and signal main loop
		old_state = newstate;
		changed = 1;
	}
}
