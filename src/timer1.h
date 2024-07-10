/*
 * timer1.h
 *
 * Author: Peter Sutton
 *
 * timer 1 skeleton
 */

#ifndef TIMER1_H_
#define TIMER1_H_

#include <stdint.h>

/* Set up our timer 
 */
void init_timer1(void);

uint16_t test;

// Piezo buzzer noise for human firing on ship
uint32_t fire_tone1(void);

// Piezo buzzer noise for computer firing on ship
uint32_t fire_tone2(void);

uint32_t note_1(void);
uint32_t note_2(void);
uint32_t note_3(void);
uint32_t note_4(void);
uint32_t note_5(void);
uint32_t note_6(void);
uint32_t note_7(void);

// Stops the Piezo Buzzer from making any noise
void sound_off(void);

#endif /* TIMER1_H_ */
