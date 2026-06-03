/********************************************
 *
 *  Name: Kaitlyn Ooi
 *  Email: kaitlyno@usc.edu
 *  Section: Wed 2pm
 *  Assignment: Lab 7 - ADC and PWM
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "adc.h"

// PWM limits (OCR2A values)
#define PWM_MIN 11 // 0.75ms pulse width
#define PWM_MAX 34 // 2.25ms pulse width
#define PWM_MID 23 // midpoint initial value

// States
#define STATE_VARIABLE 0
#define STATE_LEFT 1
#define STATE_RIGHT 2

void timer2_init(void);

int main(void)
{
    // Initialize the LCD
    lcd_init();

    // Initialize the ADC
    adc_init();

    // Initialize TIMER2 for Fast PWM
    timer2_init();

    // Set Port B bit 3 (D11/OC2A) as output
    DDRB |= (1 << 3);

    // Write splash screen and delay for 1 second
    lcd_moveto(0, 0);
    lcd_stringout("  Lab 7-ADC  ");
    lcd_moveto(1, 0);
    lcd_stringout("   Kaitlyn Ooi   ");
    _delay_ms(1000);
    lcd_writecommand(0x01);

    unsigned char state = STATE_VARIABLE;
    unsigned char last_pwm = 255;
    unsigned char pwm_val;
    unsigned char adc_btn;

    unsigned char adc_pot = 0;
    char buf[6];

    /*
    Use this "while (1)" loop ONLY for doing Tasks 2 and 3
    unsigned char adc_val;
    char buf[5];
    while (1) {
        adc_val = adc_sample(0);  // ch0 = buttons; ch1 = pot
        snprintf(buf, 5, "%4d", adc_val);
        lcd_moveto(0, 0);
        lcd_stringout(buf);
    }
*/

    while (1) // Loop forever
    {
        // Read the ADC value from the buttons (channel 0)
        adc_btn = adc_sample(0);
        // Check buttons and determine state
        if (adc_btn < 10)
        {
            state = STATE_RIGHT; // RIGHT 0
        }
        else if (adc_btn >= 140 && adc_btn < 180)
        {
            state = STATE_LEFT; // LEFT 154
        }
        else if (adc_btn >= 185 && adc_btn < 225)
        {
            state = STATE_VARIABLE; // SELECT 205
        }

        // If SELECT button pressed read potentiometer ADC channel
        if (state == STATE_VARIABLE)
        {
            // Read potentiometer (channel 1) and calculate PWM value
            adc_pot = adc_sample(1);
            pwm_val = PWM_MAX - ((unsigned int)adc_pot * (PWM_MAX - PWM_MIN)) / 255;
        }
        // If RIGHT or LEFT button pressed, move servo accordingly
        else if (state == STATE_LEFT)
        {
            pwm_val = PWM_MAX;
        }
        else
        {
            pwm_val = PWM_MIN;
        }

        // Update OCR2A
        OCR2A = pwm_val;

        // Display the PWM value on the LCD
        if (pwm_val != last_pwm)
        {
            snprintf(buf, 6, "%4d", pwm_val);
            lcd_moveto(0, 0);
            lcd_stringout("PWM:");
            lcd_moveto(0, 4);
            lcd_stringout(buf);
            last_pwm = pwm_val;
        }
        _delay_ms(10);
    }
    return 0; /* never reached */
}

/*
  timer2_init - Initialize Timer/Counter2 for Fast PWM
*/
void timer2_init(void)
{
    // Add code to initialize TIMER2
    // Set Timer2 to Fast PWM mode
    TCCR2A |= (0b11 << WGM20);
    TCCR2A |= (0b10 << COM2A0);
    OCR2A = PWM_MID;
    // Set prescaler to 1024 so PWM runs at a suitable frequency
    TCCR2B |= (0b111 << CS20);
}