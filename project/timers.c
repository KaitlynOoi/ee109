#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "project.h"
#include "timers.h"

#define MODE_IDLE 0
#define MODE_ECHO 1
#define MODE_STOPWATCH 2

#define MIN_ECHO_COUNT 40

static volatile uint8_t mode;
static volatile uint8_t started;

static volatile uint16_t buzz_count = 0;
static volatile uint8_t buzzing = 0;
static volatile uint8_t tone_index = 0;
static volatile uint8_t direction_flag = 0;

void range_init(void)
{
  DDRD |= (1 << PD3);
  DDRD &= ~(1 << PD2);
  PORTD &= ~(1 << PD3);

  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  TCNT1 = 0;

  PCICR &= ~(1 << PCIE2);
  PCMSK2 &= ~(1 << PCINT18);

  mode = MODE_IDLE;
  started = 0;
}

void range_start_measurement(void)
{
  done = 0;
  valid = 0;
  timeout = 0;
  pulse = 0;
  started = 0;
  mode = MODE_ECHO;

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 60000;
  TIFR1 = (1 << OCF1A);
  TIMSK1 = (1 << OCIE1A);
  TCCR1B = (1 << CS11);

  PCIFR = (1 << PCIF2);
  PCMSK2 |= (1 << PCINT18);
  PCICR |= (1 << PCIE2);

  PORTD &= ~(1 << PD3);
  _delay_us(2);
  PORTD |= (1 << PD3);
  _delay_us(10);
  PORTD &= ~(1 << PD3);
}

void stopwatch_start(void)
{
  time = 0;
  timeout = 0;
  mode = MODE_STOPWATCH;

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 6249;
  TIFR1 = (1 << OCF1A);
  TIMSK1 = (1 << OCIE1A);
  TCCR1B = (1 << WGM12) | (1 << CS12);
}

void stopwatch_stop(void)
{
  TIMSK1 = 0;
  TCCR1B = 0;
  mode = MODE_IDLE;
}

uint16_t pulse_to_mm(uint16_t pulse_count)
{
  return (uint16_t)(((uint32_t)pulse_count * 5UL + 29UL) / 58UL);
}

void start_buzzer_sequence(uint8_t above)
{
  TCCR0B = 0; // stop timer
  TCNT0 = 0;  // reset

  direction_flag = above;
  tone_index = 0;
  buzz_count = 0;
  buzzing = 1;

  DDRB |= (1 << PB5); // D13

  TCCR0A = (1 << WGM01);              // CTC mode
  TCCR0B = (1 << CS01) | (1 << CS00); // prescaler 64
  TIMSK0 = (1 << OCIE0A);

  // FIRST tone
  if (direction_flag)
    OCR0A = 250; // LOW tone
  else
    OCR0A = 50; // HIGH tone
}

ISR(PCINT2_vect)
{
  if (mode != MODE_ECHO)
    return;

  if (PIND & (1 << PD2))
  {
    TCNT1 = 0;
    started = 1;
  }
  else
  {
    if (started)
    {
      if (TCNT1 < MIN_ECHO_COUNT)
      {
        started = 0;
        TCNT1 = 0;
        return;
      }

      pulse = TCNT1;
      valid = 1;
      done = 1;

      PCICR &= ~(1 << PCIE2);
      PCMSK2 &= ~(1 << PCINT18);
      TIMSK1 = 0;
      TCCR1B = 0;
      mode = MODE_IDLE;
      started = 0;
    }
  }
}

ISR(TIMER1_COMPA_vect)
{
  if (mode == MODE_ECHO)
  {
    valid = 0;
    done = 1;
    timeout = 1;

    PCICR &= ~(1 << PCIE2);
    PCMSK2 &= ~(1 << PCINT18);
    TIMSK1 = 0;
    TCCR1B = 0;
    mode = MODE_IDLE;
    started = 0;
  }
  else if (mode == MODE_STOPWATCH)
  {
    time++;

    if (time > 100)
    {
      time = 100;
      timeout = 1;
      stopwatch_stop();
    }
  }
}

ISR(TIMER0_COMPA_vect)
{
  if (!buzzing)
    return;

  PORTB ^= (1 << PB5); // toggle pin

  buzz_count++;

  if (buzz_count > 500) // duration per tone
  {
    buzz_count = 0;
    tone_index++;
    // After 3 tones, stop buzzing
    if (tone_index >= 3)
    {
      buzzing = 0;
      TCCR0B = 0;
      PORTB &= ~(1 << PB5);
      return;
    }

    // CHANGE FREQUENCY
    if (direction_flag)
    {
      if (tone_index == 1)
        OCR0A = 120;
      else if (tone_index == 2)
        OCR0A = 50;
    }
    else
    {
      if (tone_index == 1)
        OCR0A = 120;
      else if (tone_index == 2)
        OCR0A = 250;
    }
  }
}