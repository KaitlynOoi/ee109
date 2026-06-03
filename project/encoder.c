#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "project.h"
#include "encoder.h"

static volatile uint8_t old; //prev state
static volatile int8_t value; //store current threshold value

void encoder_set_led(uint8_t color)
{
  // Common anode RGB LED: 0 = on, 1 = off 
  DDRC |= (1 << PC1) | (1 << PC2) | (1 << PC3);

  if (color == LED_RED)
  {
    PORTC &= ~(1 << PC1);
    PORTC |= (1 << PC2) | (1 << PC3);
  }
  else if (color == LED_GREEN)
  {
    PORTC &= ~(1 << PC2);
    PORTC |= (1 << PC1) | (1 << PC3);
  }
  else
  {
    PORTC &= ~(1 << PC3);
    PORTC |= (1 << PC1) | (1 << PC2);
  }
}

void encoder_init(void)
{
  uint8_t bits, a, b;

  DDRC &= ~((1 << PC4) | (1 << PC5)); 
  PORTC |= (1 << PC4) | (1 << PC5);

  bits = PINC;
  a = bits & (1 << PC4);
  b = bits & (1 << PC5);
// State encoding: 0 = 00, 1 = 01, 2 = 10, 3 = 11 (b is MSB)
  if (!b && !a)
    old = 0;
  else if (!b && a)
    old = 1;
  else if (b && !a)
    old = 2;
  else
    old = 3;

  value = THRESHOLD_DEFAULT;
  limit = THRESHOLD_DEFAULT;
  changed = 1;

  PCICR |= (1 << PCIE1);  //enables the Port C pin-change interrupt group.
  PCMSK1 |= (1 << PCINT12) | (1 << PCINT13); //turns on interrupts for PC4 and PC5.
}

ISR(PCINT1_vect)
{
  uint8_t bits, a, b, new_state;

  bits = PINC;
  a = bits & (1 << PC4);
  b = bits & (1 << PC5);

  if (!b && !a)
    new_state = 0;
  else if (!b && a)
    new_state = 1;
  else if (b && !a)
    new_state = 2;
  else
    new_state = 3;

  if (new_state != old)
  {
    if (old == 0)
    {
      if (new_state == 1)
        value++;
      else if (new_state == 2)
        value--;
    }
    else if (old == 1)
    {
      if (new_state == 0)
        value--;
      else if (new_state == 3)
        value++;
    }
    else if (old == 2)
    {
      if (new_state == 3)
        value--;
      else if (new_state == 0)
        value++;
    }
    else
    {
      if (new_state == 2)
        value++;
      else if (new_state == 1)
        value--;
    }

    if (value > 99)
      value = 99;
    else if (value < 1)
      value = 1;

    limit = (uint8_t)value; // update global limit variable
    changed = 1;
    old = new_state;
  }
}