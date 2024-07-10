/*
 * timer2.c
 *
 * Author: Peter Sutton
 *
 * timer 2 skeleton
 */

#include "timer2.h"
#include <avr/io.h>
#include <avr/interrupt.h>
uint8_t digit;
uint8_t seven_seg[10] = { 63,6,91,79,102,109,125,7,127,111};
seven_seg_display_value = 0;
extern uint8_t paused;
/* Set up timer 2
 */

/* Set up timer 2 to generate an interrupt every 10ms. 
 * We will divide the clock by 64 and count up to 124.
 * We will therefore get an interrupt every 64 x 125
 * clock cycles, i.e. every 1 milliseconds with an 8MHz
 * clock. 
 * The counter will be reset to 0 when it reaches it's
 * output compare value.
 */
void init_timer2(void)
{

	/* Set port C (all pins) to be outputs */
	DDRC = 0xFF;

	/* Set port D, pin 7 to be an output */
	DDRD |= (1 << 7);

	digit = 0;
	
	/* Clear the timer */
	TCNT2 = 0;

	/* Set the output compare value to be 124 */
	OCR2A = 78; // calculated via formula f = fcpu/(N(OCR + 1))
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR2A = (1 << WGM01);
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK2 |= (1 << OCIE2A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR2 = (1 << OCF2A);

	stagnant_ssd_value = 1;

}

ISR(TIMER2_COMPA_vect)
{
	digit = 1-digit;
	if (seven_seg_display_value == 0) {
		PORTC = 0;
	} else {
		if (! stagnant_ssd_value) {
	if (!paused) {
		seven_seg_display_value -= 1;
	}
		}
	if (seven_seg_display_value < 1000) {
		if (digit) {
			PORTD |= (1 << 7);
			PORTC = seven_seg[(seven_seg_display_value/100)%10] | (1 << 7);
		} else {
			PORTD &= ~(1 << 7); 
			PORTC = seven_seg[(seven_seg_display_value/10)%10];
		}
	} else {
		if (digit) {
			PORTD |= (1 << 7);
			PORTC = seven_seg[(seven_seg_display_value/1000)%10];
		} else {
			PORTD &= ~(1 << 7); 
			PORTC = seven_seg[(seven_seg_display_value/100)%10];
		}
	}
	}
}
	
	