#ifndef PROJECT_H
#define PROJECT_H

#include <stdint.h>

#define THRESHOLD_DEFAULT 20

#define ST_IDLE 0
#define ST_FIRST_MEAS 1
#define ST_WAIT_SECOND 2
#define ST_SECOND_MEAS 3

extern volatile uint8_t state;

extern volatile uint16_t dist1;
extern volatile uint16_t dist2;
extern volatile uint16_t time;
extern volatile int16_t speed;

extern volatile uint8_t error;
extern volatile uint8_t shown;

extern volatile uint8_t limit;
extern volatile uint8_t changed;
extern volatile uint8_t far;
extern volatile uint8_t done;
extern volatile uint8_t valid;
extern volatile uint8_t timeout;
extern volatile uint16_t pulse;

extern volatile uint8_t ready;
extern volatile uint8_t hold;

#endif