#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

void lcd_writenibble(unsigned char);

#define DATA_BITS ((1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4))
#define CTRL_BITS ((1 << PB1) | (1 << PB0))

void lcd_init(void)
{
  // Set the DDR register bits for ports B and D
  DDRB |= (1 << PB0) | (1 << PB1);                           // RS and E pin
  DDRB &= ~(1 << PB2);                                       // Set PB2 as input
  DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7); // Set PD4-PD7 as outputs

  _delay_ms(15);

  lcd_writenibble(0x30); // Use lcd_writenibble to send 0b0011
  _delay_ms(5);

  lcd_writenibble(0x30); // Use lcd_writenibble to send 0b0011
  _delay_us(120);

  lcd_writenibble(0x30); // Use lcd_writenibble to send 0b0011, no delay needed

  lcd_writenibble(0x20); // Use lcd_writenibble to send 0b0010
  _delay_ms(2);

  lcd_writecommand(0x28); // Function Set: 4-bit interface, 2 lines

  lcd_writecommand(0x0f); // Display and cursor on
}

void lcd_moveto(unsigned char row, unsigned char col)
{
  unsigned char pos;
  if (row == 0)
  {
    pos = 0x80 + col; // 1st row locations start at 0x80
  }
  else
  {
    pos = 0xc0 + col; // 2nd row locations start at 0xc0
  }
  lcd_writecommand(pos); // Send command
}

void lcd_stringout(char *str)
{
  int i = 0;
  while (str[i] != '\0')
  {                        // Loop until next charater is NULL byte
    lcd_writedata(str[i]); // Send the character
    i++;
  }
}

void lcd_writecommand(unsigned char cmd)
{
  /* Clear PB0 to 0 for a command transfer */
  PORTB &= ~(1 << PB0); // Clear RS for command write
  /* Call lcd_writenibble to send UPPER four bits of "cmd" argument */
  lcd_writenibble(cmd); // Send upper nibble
  /* Call lcd_writenibble to send LOWER four bits of "cmd" argument */
  lcd_writenibble(cmd << 4); // Send lower nibble
  /* Delay 2ms */
  _delay_ms(2);
}

void lcd_writedata(unsigned char dat)
{
  /* Set PB0 to 1 for a data transfer */
  PORTB |= (1 << PB0); // Set RS for data write
  /* Call lcd_writenibble to send UPPER four bits of "dat" argument */
  lcd_writenibble(dat); // Send upper nibble
  /* Call lcd_writenibble to send LOWER four bits of "dat" argument */
  lcd_writenibble(dat << 4); // Send lower nibble
  /* Delay 2ms */
  _delay_ms(2);
}

void lcd_writenibble(unsigned char lcdbits)
{
  /* Load PORTD, bits 7-4 with bits 7-4 of "lcdbits" */
  PORTD = (PORTD & 0x0F) | (lcdbits & 0xF0);
  /* Make E signal (PB1) go to 1 and back to 0 */
  PORTB |= (1 << PB1);  // Set E to 1
  PORTB |= (1 << PB1);  // Make sure E is >230ns
  PORTB &= ~(1 << PB1); // Set E to 0
}
