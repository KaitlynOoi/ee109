#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include "serial.h"

volatile uint8_t rx_ready = 0;
char rx_buf[6];
int16_t remote_speed = 0;

static uint8_t rx_index = 0;
static uint8_t rx_started = 0;

void serial_init(void)
{
  UBRR0 = 103; // 9600 buad at 16MHz
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void send_speed(int16_t s)
{
  char buf[10];

  snprintf(buf, sizeof(buf), "@%d$", s);

  for (int i = 0; buf[i] != '\0'; i++)
  {
    while (!(UCSR0A & (1 << UDRE0)))
      ;
    UDR0 = buf[i];
  }
}

ISR(USART_RX_vect)
{
  char c = UDR0;

  if (c == '@')
  {
    rx_started = 1;
    rx_index = 0;
    rx_ready = 0;
    return;
  }

  if (!rx_started)
    return;

  if (c == '$')
  {
    if (rx_index > 0)
    {
      rx_buf[rx_index] = '\0';
      rx_ready = 1;
    }
    rx_started = 0;
    return;
  }

  if ((c >= '0' && c <= '9') || c == '-')
  {
    if (rx_index < 5)
      rx_buf[rx_index++] = c;
    else
      rx_started = 0;
  }
  else
  {
    rx_started = 0;
  }
}