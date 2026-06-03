#include <avr/io.h>

#include "adc.h"

void adc_init(void)
{
    // Initialize the ADC, selects AVcc as ref voltage, left adjust result, and sets prescaler to 128
    ADMUX = (1 << REFS0) | (1 << ADLAR);
    // Enable the ADC and set the prescaler to 128 (for 16 MHz clock, this gives 125 kHz ADC clock)
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint8_t adc_sample(uint8_t channel)
{
    // Set ADC input mux bits to 'channel' value
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Start the ADC conversion
    ADCSRA |= (1 << ADSC);
    // Wait for the conversion to complete
    while (ADCSRA & (1 << ADSC))
        ;
    // Return the 8-bit result
    return ADCH;
}
