#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>

void range_init(void);
void range_start_measurement(void);

void stopwatch_start(void);
void stopwatch_stop(void);

void play_tone(uint16_t freq);
void stop_tone(void);

void start_buzzer_sequence(uint8_t above);

uint16_t pulse_to_mm(uint16_t pulse_count);

#endif