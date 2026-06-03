#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "adc.h"
#include "project.h"
#include "timers.h"
#include "encoder.h"
#include "serial.h"

// LCD buttons
#define ADC_RIGHT 0
#define ADC_LEFT 154
#define ADC_TOL 20

#define BTN_NONE 0
#define BTN_RIGHT 1
#define BTN_LEFT 2
// state machine
volatile uint8_t state = ST_IDLE;
// Measurement values
volatile uint16_t dist1 = 0;
volatile uint16_t dist2 = 0;
volatile uint16_t time = 0;
volatile int16_t speed = 0;
// Status flags
volatile uint8_t error = 0;
volatile uint8_t shown = 0;
// Speed limit threshold
volatile uint8_t limit = THRESHOLD_DEFAULT;
volatile uint8_t changed = 0;
// Measurement flags
volatile uint8_t far = 0;
volatile uint8_t done = 0;
volatile uint8_t valid = 0;
volatile uint8_t timeout = 0;
volatile uint16_t pulse = 0;

volatile uint8_t ready = 0;
volatile uint8_t hold = 0;
volatile uint8_t remote_updated = 0;
// Update servo position based on elapsed time.
void update_servo(void)
{
  uint8_t min = 12; // left
  uint8_t max = 35; // right

  OCR2A = max - ((time * (max - min)) / 100);
}
// Reads ADC value and determines which button is pressed.
static uint8_t read_button(void)
{
  uint8_t val = adc_sample(0);

  if (val < ADC_RIGHT + ADC_TOL)
    return BTN_RIGHT;
  else if ((val > ADC_LEFT - ADC_TOL) && (val < ADC_LEFT + ADC_TOL))
    return BTN_LEFT;
  else
    return BTN_NONE;
}
// Display distance in mm
static void print_distance(uint16_t mm)
{
  char buf[8];
  snprintf(buf, sizeof(buf), "%3u.%1u", mm / 10, mm % 10);
  lcd_stringout(buf);
}
// Display time in seconds
static void print_time(uint16_t t)
{
  char buf[8];
  snprintf(buf, sizeof(buf), "%2u.%1u", t / 10, t % 10);
  lcd_stringout(buf);
}
// Display speed
static void print_speed(int16_t s)
{
  char buf[8];

  if (s < 0)
  {
    s = -s;
    snprintf(buf, sizeof(buf), "-%3u.%1u", (uint16_t)(s / 10), (uint16_t)(s % 10));
  }
  else
  {
    snprintf(buf, sizeof(buf), " %3u.%1u", (uint16_t)(s / 10), (uint16_t)(s % 10));
  }

  lcd_stringout(buf);
}
// Display speed limit threshold
static void print_limit(uint8_t x)
{
  char buf[4];
  snprintf(buf, sizeof(buf), "%2u", x);
  lcd_stringout(buf);
}
// Check if speed exceeds limit
static uint8_t above_limit(void)
{
  uint16_t mag;

  if (speed < 0)
    mag = (uint16_t)(-speed);
  else
    mag = (uint16_t)speed;

  return (mag > (limit * 10));
}
// Display error message for out-of-range measurements
static void show_error(void)
{
  lcd_moveto(0, 0);
  lcd_stringout(">400            ");

  lcd_moveto(1, 0);
  print_limit(limit);
  lcd_writedata(' ');
  print_limit(limit);

  lcd_moveto(1, 10);

  if (remote_updated)
  {
    print_speed(remote_speed);
  }
  else
  {
    lcd_stringout("    ");
  }
}
// Main display function to update LCD based on current state and flags
static void display(void)
{
  // If there was an error and the object is far, show error message
  if (far == 1)
  {
    if (!shown)
    {
      lcd_writecommand(1);
      _delay_ms(2);
      show_error();
      shown = 1;
    }
    return;
  }

  if (hold)
    return;

  lcd_moveto(0, 0);
  print_distance(dist1);
  lcd_writedata(' ');
  print_distance(dist2);
  lcd_writedata(' ');
  print_time(time);

  lcd_moveto(1, 0);

  if (ready)
    print_speed(speed);
  else
    print_speed(0);

  lcd_writedata(' ');
  print_limit(limit);
  lcd_moveto(1, 10);

  if (remote_updated)
  {
    print_speed(remote_speed);
  }
  else
  {
    lcd_stringout("    ");
  }
}
// Start first measurement sequence
static void start_first(void)
{
  far = 0;
  error = 0;
  shown = 0;
  dist1 = 0;
  dist2 = 0;
  time = 0;
  speed = 0;
  ready = 0;
  timeout = 0;
  hold = 0;

  encoder_set_led(LED_BLUE);
  range_start_measurement();
  state = ST_FIRST_MEAS;
}
// Start second measurement sequence
static void start_second(void)
{
  far = 0;
  timeout = 0;
  done = 0;
  valid = 0;
  pulse = 0;
  error = 0;
  shown = 0;

  encoder_set_led(LED_BLUE);

  stopwatch_stop();
  range_start_measurement();
  state = ST_SECOND_MEAS;
}
// Main function
int main(void)
{
  uint8_t btn;
  uint16_t mm;

  lcd_init();
  adc_init();
  encoder_init();
  range_init();

  uint8_t eeprom_val = eeprom_read_byte((void *)0);

  if (eeprom_val >= 1 && eeprom_val <= 99)
    limit = eeprom_val;
  else
    limit = THRESHOLD_DEFAULT;

  DDRB |= (1 << PB3); // servo pin

  TCCR2A = (1 << COM2A1) | (1 << WGM20) | (1 << WGM21); // Fast PWM
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);     // prescaler 1024

  OCR2A = 25; // initial
              // serial communication
  serial_init();

  DDRB |= (1 << PB4);
  PORTB &= ~(1 << PB4);

  sei();

  lcd_writecommand(1);
  _delay_ms(2);
  lcd_moveto(0, 0);
  lcd_stringout("Kaitlyn Ooi");
  lcd_moveto(1, 0);
  lcd_stringout("Speedometer");
  _delay_ms(1000);

  lcd_writecommand(1);
  _delay_ms(2);

  encoder_set_led(LED_BLUE);

  while (1)
  {

    update_servo(); // continuously update servo position based on elapsed time
    static uint8_t once = 0;
    // handle incoming serial speed data
    if (rx_ready)
    {
      int16_t new_speed;

      sscanf(rx_buf, "%d", &new_speed);

      remote_speed = new_speed;
      rx_ready = 0;

      remote_updated = 1;

      // trigger buzzer if remote speed exceeds limit
      if (abs(remote_speed) > (limit * 10))
        start_buzzer_sequence(1);
      else
        start_buzzer_sequence(0);
    }

    btn = read_button();
    // save updated limit to EEPROM
    if (changed)
    {
      changed = 0;
      eeprom_update_byte((void *)0, limit);

      if (ready && state == ST_IDLE && !hold)
      {
        if (above_limit())
          encoder_set_led(LED_RED);
        else
          encoder_set_led(LED_GREEN);
      }
    }

    if (state == ST_IDLE)
    {
      if (btn == BTN_LEFT)
      {
        _delay_ms(150);
        start_first();
      }
    }
    else if (state == ST_FIRST_MEAS)
    {
      if (done)
      {
        done = 0;
        mm = pulse_to_mm(pulse);

        if (!valid)
        {
          error = 1;
          far = 1;
          shown = 0;
          ready = 0;
          hold = 0;
          encoder_set_led(LED_BLUE);
          state = ST_IDLE;
        }
        else if (mm > 4000)
        {
          far = 1;
          error = 0;
          shown = 0;
          ready = 0;
          hold = 0;
          encoder_set_led(LED_BLUE);
          state = ST_IDLE;
        }
        else
        {
          far = 0;
          error = 0;
          shown = 0;
          dist1 = mm;
          time = 0;
          stopwatch_start();
          state = ST_WAIT_SECOND;
        }
      }
    }
    else if (state == ST_WAIT_SECOND)
    {
      if (timeout)
      {
        ready = 0;
        hold = 1;
        encoder_set_led(LED_BLUE);
        state = ST_IDLE;
      }
      else if (btn == BTN_RIGHT)
      {
        _delay_ms(150);
        start_second();
      }
    }
    else if (state == ST_SECOND_MEAS)
    {
      if (done)
      {
        done = 0;
        stopwatch_stop();
        mm = pulse_to_mm(pulse);

        if (!valid)
        {
          error = 1;
          far = 1;
          shown = 0;
          ready = 0;
          hold = 0;
          encoder_set_led(LED_BLUE);
          state = ST_IDLE;
        }
        else if (mm > 4000)
        {
          far = 1;
          error = 0;
          shown = 0;
          ready = 0;
          hold = 0;
          encoder_set_led(LED_BLUE);
          state = ST_IDLE;
        }
        else
        {
          far = 0;
          error = 0;
          shown = 0;
          dist2 = mm;

          if (time == 0)
          {
            ready = 0;
            hold = 1;
            encoder_set_led(LED_BLUE);
            state = ST_IDLE;
          }
          else
          {
            speed = ((int32_t)dist2 - dist1) * 10 / time;
            ready = 1;
            hold = 0;
            error = 0;
            shown = 0;

            send_speed(speed);

            if (above_limit())
              encoder_set_led(LED_RED);
            else
              encoder_set_led(LED_GREEN);

            state = ST_IDLE;
          }
        }
      }
    }

    display();
  }
}
