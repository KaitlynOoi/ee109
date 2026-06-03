/********************************************
 *
 *  Name: Kaitlyn Ooi
 *  Email: kaitlyno@usc.edu
 *  Section: wed 2pm
 *  Assignment: Lab 5 - Timers
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lcd.h"

void debounce(uint8_t);
void timer1_init(void);
void timer1_start(void);
void timer1_stop(void);

volatile uint8_t tens, ones, tenths;
volatile uint8_t time_changed; // Flag that the time has changed

void debounce(uint8_t);

enum states
{
	PAUSE,
	RUN,
	LAPPED,
	PAUSE_DB,
	RUN_DB,
	LAPPED_DB
};

void timer1_init(void)
{
	TCCR1B |= (1 << WGM12);	 // Set for CTC mode using OCR1A for the modulus
	TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt

	OCR1A = 6250; // Set the counter modulus correctly
}

void timer1_start(void)
{
	TCCR1B |= (1 << CS12); // Set prescaler correctly, starts timer
}

void timer1_stop(void)
{
	TCCR1B &= ~(1 << CS12); // Set prescaler for none, stops timer
}

int main(void)
{

	uint8_t state = PAUSE; // Start off in the PAUSE state

	uint8_t last_ss_button = 0;
	uint8_t last_lr_button = 0;

	// Initialize the LCD and TIMER1
	lcd_init();
	timer1_init();

// This code is only executed for the "timer_test" of TASK 3
#ifdef TASK3
	DDRC |= (1 << PC5);
	timer1_start();
	sei();
	while (1)
	{
	}
#endif

	// Enable pull-ups for buttons
	PORTC |= ((1 << PC2) | (1 << PC4));

	// Show the splash screen
	lcd_init();
	lcd_moveto(0, 0);
	lcd_stringout("Kaitlyn Ooi");
	lcd_moveto(1, 0);
	lcd_stringout("Lab 5 - Timers");
	_delay_ms(1000);
	lcd_writecommand(0x01);
	// Add code here for Task 1, writing the splashscreen

	lcd_moveto(0, 0); // Show 0.0 for the initial time
	lcd_stringout(" 0.0");

	// Enable interrupts
	sei(); // Enable interrupts

	while (1)
	{ // Loop forever
		uint8_t ss_button, lr_button;

		// Read the buttons
		ss_button = (PINC & (1 << PC2)) == 0; // Read the Start_Stop button
		lr_button = (PINC & (1 << PC4)) == 0; // Read the Lapped_Reset button

		uint8_t ss_rising = (ss_button && !last_ss_button);
		uint8_t lr_rising = (lr_button && !last_lr_button);

		// Determine which state we are in
		if (state == PAUSE)
		{ // PAUSE state
			if (ss_button)
			{

				timer1_start(); // Start the timer
				state = RUN_DB; // Move to RUN state via debouncing
												// debounce(PC2);	// Debounce the Start_Stop button
			}
			else if (lr_button)
			{
				TCNT1 = 0; // Clear the timer count to zero
				tens = ones = tenths = 0;
				lcd_moveto(0, 0);
				lcd_stringout(" 0.0");
			}
		}
		else if (state == RUN)
		{ // RUN state
			if (ss_button)
			{
				timer1_stop();		// Stop the timer
				state = PAUSE_DB; // Move to the PAUSE state
			}
			else if (lr_button)
			{
				state = LAPPED_DB; // Move to the LAPPED state
													 // debounce(PC4);	// Debounce the Lap_Reset button
			}
		}

		else if (state == LAPPED)
		{ // LAPPED state
			if (ss_button || lr_button)
			{									// Either button takes us to RUN
				state = RUN_DB; // Move to the RUN state
												// if (ss_button)
												//		debounce(PC2); // Debounce the Start_Stop button
												//	else
												//	debounce(PC4); // Debounce the Lap_Reset button
			}
		}
		else if (state == RUN_DB)
		{
			if (!((PINC & (1 << PC2)) == 0) && !((PINC & (1 << PC4)) == 0)) // wait until button released
			{
				state = RUN;
			}
			_delay_ms(5); // debounce the Start_Stop and Lap_Reset buttons
		}

		else if (state == PAUSE_DB)
		{

			if (!((PINC & (1 << PC2)) == 0))
			{
				state = PAUSE;
			}
			_delay_ms(5); // debounce the Start_Stop button
		}

		else if (state == LAPPED_DB)
		{
			// Wait until the Lap button is released, then show LAPPED state.
			if (!((PINC & (1 << PC4)) == 0))
			{
				state = LAPPED;
			}
			_delay_ms(5); // debounce the Lap_Reset button
		}
		// If in RUN state and time has changed write time to LCD
		if (time_changed && (state == RUN || state == RUN_DB))
		{
			time_changed = 0;
			lcd_moveto(0, 0);
			if (tens != 0) // Get rid of leading zeros
				lcd_writedata(tens + '0');
			else
				lcd_writedata(' ');
			lcd_writedata(ones + '0');
			lcd_writedata('.');
			lcd_writedata(tenths + '0');
		}
	}

	return 0; /* never reached */
}

/* ----------------------------------------------------------------------- */

void debounce(uint8_t bit)
{
	(void)bit;
	_delay_ms(5);
	while ((PINC & (1 << bit)) == 0)
		;
	_delay_ms(5);
}

/* ----------------------------------------------------------------------- */

ISR(TIMER1_COMPA_vect)
{
#ifdef TASK3
	// For Task 3, the ISR only executes this code to flip the PC5 bit
	PORTC ^= (1 << PC5);
#else
	// Increment the time
	tenths++;
	if (tenths == 10)
	{
		tenths = 0;
		ones++;
		if (ones == 10)
		{
			ones = 0;
			tens++;
			if (tens == 6)
			{
				tenths = 0;
				ones = 0;
				tens = 0;
			}
		}
	}
	time_changed = 1;
#endif
}
