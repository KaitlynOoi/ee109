/********************************************
 *
 *  Name: Kaitlyn Ooi
 *  Email: kaitlyno@usc.edu
 *  Lab section: Wed 2pm
 *  Assignment: Lab 3 - Arduino Input and Output
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{

    // Initialize appropriate DDR registers
    DDRC |= (1 << 3);
    // Initialize the LED output to 0
    PORTC &= ~(1 << 3);
    // Enable the pull-up resistors for the 3 button inputs
    DDRD &= ~((1 << 3) | (1 << 4) | (1 << 5));
    PORTD |= (1 << 3) | (1 << 4) | (1 << 5);
    // Loop forever
    while (1)
    {
        //  Read the state of the three buttons to
        //  see how many are presssed.
        int num = 0;

        if ((PIND & (1 << 3)) == 0)
            num++;
        if ((PIND & (1 << 4)) == 0)
            num++;
        if ((PIND & (1 << 5)) == 0)
            num++;

        // If less than 2 pressed blink the LED at 5Hz rate
        if (num < 2)
        {
            PORTC |= (1 << 3);
            _delay_ms(100);
            PORTC &= ~(1 << 3);
            _delay_ms(100);
        }
        // If 2 or more pressed blink the LED at 1Hz rate
        else
        {
            PORTC |= (1 << 3);
            _delay_ms(500);
            PORTC &= ~(1 << 3);
            _delay_ms(500);
        }
    }
}

return 0; /* never reached */
