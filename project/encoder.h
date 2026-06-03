#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

//
#define LED_BLUE 0
#define LED_RED 1
#define LED_GREEN 2

// Initialize rotary encoder
void encoder_init(void);

// Set RGB LED color
void encoder_set_led(uint8_t color);

#endif