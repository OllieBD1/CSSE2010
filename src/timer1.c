// /*
//  * timer1.c
//  *
//  * Author: Peter Sutton
//  *
//  * timer 1 skeleton
//  */

#include "timer1.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

uint16_t freq_to_clock_period(uint16_t freq) {
	return (1000000UL / freq);	// UL makes the constant an unsigned long (32 bits)
								// and ensures we do 32 bit arithmetic, not 16
}

// Return the width of a pulse (in clock cycles) given a duty cycle (%) and
// the period of the clock (measured in clock cycles)
uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod) {
	return (dutycycle * clockperiod) / 100;
}

/* Set up timer 1
 */
void init_timer1(void)
{
	TCNT1 = 0;

	// Make pin OC1B be an output (port D, pin 4)
	DDRD |= (1<<4);
	
	// Set up timer/counter 1 for Fast PWM, counting from 0 to the value in OCR1A
	// before reseting to 0. Count at 1MHz (CLK/8).
	// Configure output OC1B to be clear on compare match and set on timer/counter
	// overflow (non-inverting mode).
	TCCR1A = (1 << COM1B1) | (0 <<COM1B0) | (1 <<WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12);
}

uint32_t fire_tone1(void) {
	uint16_t freq = 800;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t fire_tone2(void) {
	uint16_t freq = 200;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_1(void) {
	uint16_t freq = 200;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_2(void) {
	uint16_t freq = 300;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_3(void) {
	uint16_t freq = 400;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_4(void) {
	uint16_t freq = 500;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_5(void) {
	uint16_t freq = 600;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_6(void) {
	uint16_t freq = 700;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

uint32_t note_7(void) {
	uint16_t freq = 800;	// Hz
	float dutycycle = 50;	// %
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
	
	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;

	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width.
	OCR1B = pulsewidth - 1;

	TCCR1B |= ((0 << CS12) | (1 << CS11) | (0 << CS10));
	return 1;
}

void sound_off(void)
{
	TCCR1B &= ~((0 << CS12) | (1 << CS11) | (0 << CS10));
}