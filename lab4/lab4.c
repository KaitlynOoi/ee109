#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

char pressed(void);

char string[] = "This is a scrolling message for 109 lab";

enum
{
    LEFT = 0,
    RIGHT = 1
};

int main(void)
{
    unsigned char state = RIGHT;
    unsigned char start = 0;
    unsigned char i, cnt = 0;

    // Turn on pull ups for input buttons
    PORTC |= (1 << 2);

    lcd_init();
    lcd_writecommand(0x01); // Clear the screen

    // show first 16 characters
    for (i = 0; i < 16; i++)
    {
        if (string[i] != '\0')
            lcd_writedata(string[i]);
        else
            lcd_writedata(' ');
    }

    while (1)
    {
        // check every 100ms
        if ((cnt & 1) == 0)
        {
            // Provide the modulus and comparison constants
            // Add code to update state based on a button press
            if (pressed())
            {
                // change direction when button pressed
                if (state == RIGHT)
                    state = LEFT;
                else
                    state = RIGHT;
            }
        }

        // update every 250 ms
        if ((cnt & 3) == 0)
        {
            // Provide the modulus and comparison constants
            // Add code to determine the starting position
            // and then display the appropriate substring

            if (state == RIGHT)
            {
                // move right in the string
                if (string[start + 16] != '\0')
                {
                    start++;
                }
            }
            else
            {
                // move left in the string
                if (start > 0)
                {
                    start--;
                }
            }

            // Move cursor to start of first row
            lcd_writecommand(0x80);

            // print 16 characters starting at start
            for (i = 0; i < 16; i++)
            {
                if (string[start + i] != '\0')
                    lcd_writedata(string[start + i]);
                else
                    lcd_writedata(' ');
            }
        }

        // reset after 20 loops
        if (cnt == 20)
        {
            cnt = 0; /* Fill in the "if" condition */
        }

        cnt++;
        _delay_ms(50); // Insert an approprite delay
    }

    return 0; /* never reached */
}

char pressed()
{
    if ((PINC & (1 << 2)) == 0)
    {
        _delay_ms(5);
        while ((PINC & (1 << 2)) == 0)
        {
        }
        return 1;
    }
    else
        return 0;
}
