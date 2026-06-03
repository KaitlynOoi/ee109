#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

extern volatile uint8_t rx_ready;
extern char rx_buf[6];
extern int16_t remote_speed;

void serial_init(void);

void send_speed(int16_t s);

#endif