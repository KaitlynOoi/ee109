/********************************************
*
*  Name:
*  Section:
*  Assignment: Lab 0 - Blink the LED on Port B, bit 5
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB |= (1 << DDB5);  /* Set PB5 for output */

    while(1) {
    PORTB |= (1 << PB5);  /* LED on */
    _delay_ms(500);         /* Wait 500 milliseconds */
    PORTB &= ~(1 << PB5); /* LED off */
    _delay_ms(500);         /* Wait 500 milliseconds */
    }

    return 0;   /* never reached */
}