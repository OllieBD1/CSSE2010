/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by Oliver Barr-David
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

/////////////////////////////// main //////////////////////////////////

int terminal_you_sunk = 6;
int terminal_i_sunk = 6;
int humans_ships_sunk = 0;
int computers_ships_sunk = 0;
int currently_cheating = 0;
int counting = 0;
int currently_playing_noise = 0;
int currently_playing_animation = 0;
int animation_over = 0;
int computer_sunk_success = 0;
int human_sunk_success = 0;
int testi = 0;
int value;

uint8_t paused = 0;
uint8_t can_play_sound = 1;

int8_t btn; // The button pushed

uint8_t x_or_y = 0;    /* 0 = x, 1 = y */
uint8_t modifier = 0;

int main(void) {
    // Setup hardware and call backs. This will turn on 
    // interrupts.
    initialise_hardware();
    
    // Show the splash screen message. Returns when display
    // is complete.
    start_screen();
    
    // Loop forever and continuously play the game.
    while(1)
    {
        new_game();
        play_game();
        handle_game_over();
        start_screen();
    }
}

void initialise_hardware(void) {

    // Set up ADC - AVCC reference, right adjust
    // Input selection doesn't matter yet - we'll swap this around in the while
    // loop below.
    ADMUX = (1<<REFS0);
    // Turn on the ADC (but don't start a conversion yet). Choose a clock
    // divider of 64. (The ADC clock must be somewhere
    // between 50kHz and 200kHz. We will divide our 8MHz clock by 64
    // to give us 125kHz.)
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);

    ledmatrix_setup();
    init_button_interrupts();
    // Setup serial port for 19200 baud communication with no echo
    // of incoming characters
    init_serial_stdio(19200, 0);
    
    init_timer0();
    init_timer1();
    init_timer2();
    
    // Turn on global interrupts
    sei();
}

void start_screen(void) {
    // Clear terminal screen and output a message
    clear_terminal();
    hide_cursor();
    set_display_attribute(FG_WHITE);
    move_terminal_cursor(10,4);
    printf_P(PSTR(" _______    ______  ________  ________  __        ________   ______   __    __  ______  _______  "));
    move_terminal_cursor(10,5);
    printf_P(PSTR("|       \\  /      \\|        \\|        \\|  \\      |        \\ /      \\ |  \\  |  \\|      \\|       \\ "));
    move_terminal_cursor(10,6);
    printf_P(PSTR("| $$$$$$$\\|  $$$$$$\\\\$$$$$$$$ \\$$$$$$$$| $$      | $$$$$$$$|  $$$$$$\\| $$  | $$ \\$$$$$$| $$$$$$$\\"));
    move_terminal_cursor(10,7);
    printf_P(PSTR("| $$__/ $$| $$__| $$  | $$      | $$   | $$      | $$__    | $$___\\$$| $$__| $$  | $$  | $$__/ $$"));
    move_terminal_cursor(10,8);
    printf_P(PSTR("| $$    $$| $$    $$  | $$      | $$   | $$      | $$  \\    \\$$    \\ | $$    $$  | $$  | $$    $$"));
    move_terminal_cursor(10,9);
    printf_P(PSTR("| $$$$$$$\\| $$$$$$$$  | $$      | $$   | $$      | $$$$$    _\\$$$$$$\\| $$$$$$$$  | $$  | $$$$$$$ "));
    move_terminal_cursor(10,10);
    printf_P(PSTR("| $$__/ $$| $$  | $$  | $$      | $$   | $$_____ | $$_____ |  \\__| $$| $$  | $$ _| $$_ | $$      "));
    move_terminal_cursor(10,11);
    printf_P(PSTR("| $$    $$| $$  | $$  | $$      | $$   | $$     \\| $$     \\ \\$$    $$| $$  | $$|   $$ \\| $$      "));
    move_terminal_cursor(10,12);
    printf_P(PSTR(" \\$$$$$$$  \\$$   \\$$   \\$$       \\$$    \\$$$$$$$$ \\$$$$$$$$  \\$$$$$$  \\$$   \\$$ \\$$$$$$ \\$$      "));
    move_terminal_cursor(10,14);
    // change this to your name and student number; remove the chevrons <>
    printf_P(PSTR("CSSE2010/7201 Project by Oliver Barr-David - 48009265"));
    
    // Output the static start screen and wait for a push button 
    // to be pushed or a serial input of 's'
    show_start_screen();

    uint32_t last_screen_update, current_time;
    last_screen_update = get_current_time();
    
    int8_t frame_number = -2*ANIMATION_DELAY;



    // Wait until a button is pressed, or 's' is pressed on the terminal
    while(1) {
        // First check for if a 's' is pressed
        // There are two steps to this
        // 1) collect any serial input (if available)
        // 2) check if the input is equal to the character 's'
        char serial_input = -1;

        if (serial_input_available())
        {
            serial_input = fgetc(stdin);
        }

        if (serial_input == '1') {
            seven_seg_display_value = 0;
            counting = 0;
        }
        else if (serial_input == '2') {
            seven_seg_display_value = 6099;
            counting = 1;
        }
        else if (serial_input == '3') {
            seven_seg_display_value = 3099;
            counting = 2;
        }
        else if (serial_input == '4') {
            seven_seg_display_value = 1599;
            counting = 3;
        }
        else if (serial_input == '5') {
            seven_seg_display_value = 509;
            counting = 4;
        }
        // Otherwise, assume mode '1'

        // If the serial input is 's', then exit the start screen
        if (serial_input == 's' || serial_input == 'S')
        {
            break;
        }
        // Next check for any button presses
        int8_t btn = button_pushed();
        if (btn != NO_BUTTON_PUSHED)
        {
            break;
        }

        // every 200 ms, update the animation
        current_time = get_current_time();
        if (current_time - last_screen_update > 200)
        {
            update_start_screen(frame_number);
            frame_number++;
            if (frame_number > ANIMATION_LENGTH)
            {
                frame_number -= ANIMATION_LENGTH+ANIMATION_DELAY;
            }
            last_screen_update = current_time;
        }
    }
}

void new_game(void) {
    // Clear the serial terminal
    clear_terminal();
    
    // Initialise the game and display
    initialise_game();
    
    // Clear a button push or serial input if any are waiting
    // (The cast to void means the return value is ignored.)
    (void)button_pushed();
    clear_serial_input_buffer();
    
    humans_ships_sunk = 0;
    computers_ships_sunk = 0;
    terminal_i_sunk = 6;
    terminal_you_sunk = 6;
}

void play_game(void) {

    uint32_t last_flash_time, current_time, cheating_time, noise_playing_time, hit_after_anim, joystick_current_time, last_joystick;
    
    last_flash_time = get_current_time();
    stagnant_ssd_value = 0;

    
	// We play the game until it's over
	while (!is_game_over(humans_ships_sunk, computers_ships_sunk)) {

		// Set the ADC mux to choose ADC0 if x_or_y is 0, ADC1 if x_or_y is 1
		if(x_or_y == 0) {
			ADMUX &= ~1;
		} else {
			ADMUX |= 1;
		}
		// Start the ADC conversion
		ADCSRA |= (1<<ADSC);
		
		while(ADCSRA & (1<<ADSC)) {
			; /* Wait until conversion finished */
		}
		value = ADC; // read the value
		
		// Next time through the loop, do the other direction
		x_or_y ^= 1;

		joystick_current_time = get_current_time();

		uint8_t modifier = 0;

		if ((value-512)>475 || (value-512)<-475){
			modifier = 20;

		}

		if (joystick_current_time - last_joystick > 200-modifier) 
		{
			
			last_joystick = get_current_time();
			if(x_or_y == 0) {
			// x direction
			if ((value - 512) > 50)
			{
				move_cursor(-1, 0);
				flash_cursor();
			}
			if ((value - 512) < -50)
			{
				move_cursor(1, 0);
				flash_cursor();
			}
		} if (!x_or_y == 0) {
			// y direction
			if ((value - 512) > 50)
			{
				move_cursor(0, 1);
				flash_cursor();
			}
			if ((value - 512) < -50)
			{
				move_cursor(0, -1);
				flash_cursor();
			}
		}
		}

        // We need to check if any button has been pushed, this will be
        // NO_BUTTON_PUSHED if no button has been pushed
        // Checkout the function comment in `buttons.h` and the implementation
        // in `buttons.c`.
        btn = button_pushed();

        char serial_input = -1;
        if (serial_input_available()) {
            serial_input = fgetc(stdin);
        }

        if (serial_input == 'p' || serial_input == 'P') {
            paused = !paused;
        }

        if (!paused) {
        move_terminal_cursor(10, 14);
        clear_to_end_of_line();  

		if (serial_input == 'q' || serial_input == 'Q') {
			can_play_sound = !can_play_sound;
		}  

        if (btn == BUTTON0_PUSHED || serial_input == 'd' || serial_input == 'D') {    
            move_cursor(1,0); // Increment right 
            flash_cursor(); // Flash the cursor
            last_flash_time = get_current_time(); // Reset flash time
        }    
        else if (btn == BUTTON1_PUSHED || serial_input == 's' || serial_input == 'S') {
            move_cursor(0,-1); // Increment down
            flash_cursor(); // Flash the cursor 
            last_flash_time = get_current_time(); // Reset the flash time
        }
        else if (btn == BUTTON2_PUSHED || serial_input == 'w' || serial_input == 'W') {
            move_cursor(0, 1); // Increment up
            flash_cursor(); // Flash the cursor
            last_flash_time = get_current_time(); // Reset the flash time
        }
        else if (btn == BUTTON3_PUSHED || serial_input == 'a' || serial_input == 'A') {
            move_cursor(-1,0); // Increment left
            flash_cursor(); // Flash the cursor 
            last_flash_time = get_current_time(); // Reset the flash time
        }
        else if (serial_input == 'c' || serial_input == 'C') {
            cheating_time = get_current_time();
            currently_cheating = cheating();
        }
        // Turn the Seven Segment Display Off
        else if (serial_input == '1') {
            seven_seg_display_value = 0;
            counting = 0;
        }
        // Display a 60 Second Timer on the Seven Segment Display that Retains Time when Switching Between '1', '2', '3', '4'
        else if (serial_input == '2') {
            if (counting == 0) {
                seven_seg_display_value = 6100;
            }
            else if (counting == 1) {
                seven_seg_display_value = 6100 - (6100 - seven_seg_display_value);
            }
            else if (counting == 2) {
                seven_seg_display_value = 6100 - (3100 - seven_seg_display_value); 
            }
            else if (counting == 3) {
                seven_seg_display_value = 6100 - (1600 - seven_seg_display_value); 
            }
            else if (counting == 4) {
                seven_seg_display_value = 6100 - (509 - seven_seg_display_value);
            }
            counting = 1;
        }
        // Display a 30 Second Timer on the Seven Segment Display that Retains Time when Switching Between '1', '2', '3', '4'
        else if (serial_input == '3') {
            if (counting == 0) {
                seven_seg_display_value = 3100;
            }
            else if (counting == 1) {
                seven_seg_display_value = 3100 - (6100 - seven_seg_display_value);
            }
            else if (counting == 2) {
                seven_seg_display_value = 3100 - (3100 - seven_seg_display_value); // REPEAT FOR ALL OTHER CASES 
            }
            else if (counting == 3) {
                seven_seg_display_value = 3100 - (1600 - seven_seg_display_value); 
            }
            else if (counting == 4) {
                seven_seg_display_value = 3100 - (509 - seven_seg_display_value);
            }
            counting = 2;
        }
        // Display a 15 Second Timer on the Seven Segment Display that Retains Time when Switching Between '1', '2', '3', '4'
        else if (serial_input == '4') {
            if (counting == 0) {
                seven_seg_display_value = 1600;
            }
            else if (counting == 1) {
                seven_seg_display_value = 1600 - (6100 - seven_seg_display_value);
            }
            else if (counting == 2) {
                seven_seg_display_value = 1600 - (3100 - seven_seg_display_value); // REPEAT FOR ALL OTHER CASES 
            }
            else if (counting == 3) {
                seven_seg_display_value = 1600 - (1600 - seven_seg_display_value); 
            }
            else if (counting == 4) {
                seven_seg_display_value = 1600 - (509 - seven_seg_display_value);
            }
            counting = 3;
        }
        // Display a 5 Second Timer on the Seven Segment Display that Retains Time when Switching Between '1', '2', '3', '4'
        else if (serial_input == '5') {
            if (counting == 0) {
                seven_seg_display_value = 509;
            }
            else if (counting == 1) {
                seven_seg_display_value = 509 - (6100 - seven_seg_display_value);
            }
            else if (counting == 2) {
                seven_seg_display_value = 509 - (3100 - seven_seg_display_value); // REPEAT FOR ALL OTHER CASES 
            }
            else if (counting == 3) {
                seven_seg_display_value = 509 - (1600 - seven_seg_display_value); 
            }
            else if (counting == 4) {
                seven_seg_display_value = 509 - (509 - seven_seg_display_value);
            }
            counting = 4; 
        }
        if ((counting) && (seven_seg_display_value <= 0) && (get_anim_count() == 0)) {
            set_anim_count(5);
            computer_fire();
            if ((computer_hit() == 1)) {
                hit_after_anim = 1;
            }
            computer_move_increment(1,0);
            if (counting == 1) {
                seven_seg_display_value = 6100; // DO THIS FOR ALL CASES
            } 
            if (counting == 2) {
                seven_seg_display_value = 3100;
            }
            if (counting == 3) {
                seven_seg_display_value = 1600;
            }
            if (counting == 4) {
                seven_seg_display_value = 509;
            }
        }
        
        else if ((serial_input == 'f' || serial_input == 'F') && get_anim_count() == 0) {

            computer_sunk_success = 0;
            human_sunk_success = 0;

            uint8_t fire_success = fire_cursor();

            uint8_t computer_fire_success = computer_fire();

             // Fire at the location of the cursor 
            if (fire_success == 1) {
                
            if ((human_hit() == 1) && (can_play_sound == 1)) {
                noise_playing_time = get_current_time();
                currently_playing_noise = note_7();
            }

            set_anim_count(5);

            if (counting == 0) {
            }
            else if (counting == 1) {
                seven_seg_display_value = 6099;
            }
            else if (counting == 2) {
                seven_seg_display_value = 3099;
            }
            else if (counting == 3) {
                seven_seg_display_value = 1599;
            }
            else if (counting == 4) {
                seven_seg_display_value = 509;
            }
                if ((human_sunk_success = you_sunk()) == 1) {

                    move_terminal_cursor(0,terminal_you_sunk);
                    terminal_you_sunk += 1;
                    printf("You Sunk My "); // 
                    if (get_computer_ship_type() == 1) { // Carrier
                        printf("Carrier");
                    }
                    if (get_computer_ship_type() == 2) { // Cruiser
                        printf("Cruiser");
                    }
                    if (get_computer_ship_type() == 3) { // Destroyer
                        printf("Destroyer");
                    }
                    if (get_computer_ship_type() == 4) { // Frigate
                        printf("Frigate");
                    }
                    if (get_computer_ship_type() == 5) { // Corvette
                        printf("Corvette");
                    }
                    if (get_computer_ship_type() == 6) { // Submarine
                        printf("Submarine");
                    }
                    computers_ships_sunk += 1;
                }
                if (! is_game_over(humans_ships_sunk, computers_ships_sunk)) {

                    computer_fire(); // Automatically fire the computer's shot

                    if ((computer_hit() == 1)) {
                        hit_after_anim = 1;
                    }

                    if (computer_fire_success == 1) {
                        if ((computer_sunk_success = i_sunk()) == 1) {
                            move_terminal_cursor(50,terminal_i_sunk);
                            terminal_i_sunk += 1;
                            printf("I Sunk Your "); 
                            if (get_human_ship_type() == 1) { // Carrier
                                printf("Carrier");
                            }
                            if (get_human_ship_type() == 2) { // Cruiser
                                printf("Cruiser");
                            }
                            if (get_human_ship_type() == 3) { // Destroyer
                                printf("Destroyer");
                            }
                            if (get_human_ship_type() == 4) { // Frigate
                                printf("Frigate");
                            }
                            if (get_human_ship_type() == 5) { // Corvette
                                printf("Corvette");
                            }
                            if (get_human_ship_type() == 6) { // Submarine
                                printf("Submarine");
                            }
                            humans_ships_sunk += 1;
                        }
                        computer_move_increment(1,0); // Move the computer's next shot to be one right, next row if will go off the board
                    }
                }
            }
        }
        
        else if ((serial_input == 'b' || serial_input == 'B') && get_anim_count() == 0) {

            computer_sunk_success = 0;
            human_sunk_success = 0;

            uint8_t fire_success = cheating_b();

            uint8_t computer_fire_success = computer_fire();

             // Fire at the location of the cursor 
            if (fire_success == 1) {
                
            if (human_hit() == 1 && (can_play_sound == 1)) {
                noise_playing_time = get_current_time();
                currently_playing_noise = note_7();
            }

            set_anim_count(5);

            if (counting == 0) {
            }
            else if (counting == 1) {
                seven_seg_display_value = 6099;
            }
            else if (counting == 2) {
                seven_seg_display_value = 3099;
            }
            else if (counting == 3) {
                seven_seg_display_value = 1599;
            }
            else if (counting == 4) {
                seven_seg_display_value = 509;
            }
                if ((human_sunk_success = you_sunk()) == 1) {

                    move_terminal_cursor(0,terminal_you_sunk);
                    terminal_you_sunk += 1;
                    printf("You Sunk My "); // 
                    if (get_computer_ship_type() == 1) { // Carrier
                        printf("Carrier");
                    }
                    if (get_computer_ship_type() == 2) { // Cruiser
                        printf("Cruiser");
                    }
                    if (get_computer_ship_type() == 3) { // Destroyer
                        printf("Destroyer");
                    }
                    if (get_computer_ship_type() == 4) { // Frigate
                        printf("Frigate");
                    }
                    if (get_computer_ship_type() == 5) { // Corvette
                        printf("Corvette");
                    }
                    if (get_computer_ship_type() == 6) { // Submarine
                        printf("Submarine");
                    }
                    computers_ships_sunk += 1;
                }
                if (! is_game_over(humans_ships_sunk, computers_ships_sunk)) {

                    computer_fire(); // Automatically fire the computer's shot

                    if ((computer_hit() == 1)) {
                        hit_after_anim = 1;
                    }

                    if (computer_fire_success == 1) {
                        if ((computer_sunk_success = i_sunk()) == 1) {
                            move_terminal_cursor(50,terminal_i_sunk);
                            terminal_i_sunk += 1;
                            printf("I Sunk Your "); 
                            if (get_human_ship_type() == 1) { // Carrier
                                printf("Carrier");
                            }
                            if (get_human_ship_type() == 2) { // Cruiser
                                printf("Cruiser");
                            }
                            if (get_human_ship_type() == 3) { // Destroyer
                                printf("Destroyer");
                            }
                            if (get_human_ship_type() == 4) { // Frigate
                                printf("Frigate");
                            }
                            if (get_human_ship_type() == 5) { // Corvette
                                printf("Corvette");
                            }
                            if (get_human_ship_type() == 6) { // Submarine
                                printf("Submarine");
                            }
                            humans_ships_sunk += 1;
                        }
                        computer_move_increment(1,0); // Move the computer's next shot to be one right, next row if will go off the board
                    }
                }
            }
        }

        else if ((serial_input == 'n' || serial_input == 'N') && get_anim_count() == 0) {

            computer_sunk_success = 0;
            human_sunk_success = 0;

            uint8_t fire_success = cheating_n();

            uint8_t computer_fire_success = computer_fire();

             // Fire at the location of the cursor 
            if (fire_success == 1) {
                
            if (human_hit() == 1 && (can_play_sound == 1)) {
                noise_playing_time = get_current_time();
                currently_playing_noise = note_7();
            }

            set_anim_count(5);

            if (counting == 0) {
            }
            else if (counting == 1) {
                seven_seg_display_value = 6099;
            }
            else if (counting == 2) {
                seven_seg_display_value = 3099;
            }
            else if (counting == 3) {
                seven_seg_display_value = 1599;
            }
            else if (counting == 4) {
                seven_seg_display_value = 509;
            }
                if ((human_sunk_success = you_sunk()) == 1) {

                    move_terminal_cursor(0,terminal_you_sunk);
                    terminal_you_sunk += 1;
                    printf("You Sunk My "); // 
                    if (get_computer_ship_type() == 1) { // Carrier
                        printf("Carrier");
                    }
                    if (get_computer_ship_type() == 2) { // Cruiser
                        printf("Cruiser");
                    }
                    if (get_computer_ship_type() == 3) { // Destroyer
                        printf("Destroyer");
                    }
                    if (get_computer_ship_type() == 4) { // Frigate
                        printf("Frigate");
                    }
                    if (get_computer_ship_type() == 5) { // Corvette
                        printf("Corvette");
                    }
                    if (get_computer_ship_type() == 6) { // Submarine
                        printf("Submarine");
                    }
                    computers_ships_sunk += 1;
                }
                if (! is_game_over(humans_ships_sunk, computers_ships_sunk)) {

                    computer_fire(); // Automatically fire the computer's shot

                    if ((computer_hit() == 1)) {
                        hit_after_anim = 1;
                    }

                    if (computer_fire_success == 1) {
                        if ((computer_sunk_success = i_sunk()) == 1) {
                            move_terminal_cursor(50,terminal_i_sunk);
                            terminal_i_sunk += 1;
                            printf("I Sunk Your "); 
                            if (get_human_ship_type() == 1) { // Carrier
                                printf("Carrier");
                            }
                            if (get_human_ship_type() == 2) { // Cruiser
                                printf("Cruiser");
                            }
                            if (get_human_ship_type() == 3) { // Destroyer
                                printf("Destroyer");
                            }
                            if (get_human_ship_type() == 4) { // Frigate
                                printf("Frigate");
                            }
                            if (get_human_ship_type() == 5) { // Corvette
                                printf("Corvette");
                            }
                            if (get_human_ship_type() == 6) { // Submarine
                                printf("Submarine");
                            }
                            humans_ships_sunk += 1;
                        }
                        computer_move_increment(1,0); // Move the computer's next shot to be one right, next row if will go off the board
                    }
                }
            }
        }

        else if ((serial_input == 'm' || serial_input == 'M') && get_anim_count() == 0) {

            computer_sunk_success = 0;
            human_sunk_success = 0;

            uint8_t fire_success = cheating_m();

            uint8_t computer_fire_success = computer_fire();

             // Fire at the location of the cursor 
            if (fire_success == 1) {
                
            if (human_hit() == 1 && (can_play_sound == 1)) {
                noise_playing_time = get_current_time();
                currently_playing_noise = note_7();
            }

            set_anim_count(5);

            if (counting == 0) {
            }
            else if (counting == 1) {
                seven_seg_display_value = 6099;
            }
            else if (counting == 2) {
                seven_seg_display_value = 3099;
            }
            else if (counting == 3) {
                seven_seg_display_value = 1599;
            }
            else if (counting == 4) {
                seven_seg_display_value = 509;
            }
                if ((human_sunk_success = you_sunk()) == 1) {

                    move_terminal_cursor(0,terminal_you_sunk);
                    terminal_you_sunk += 1;
                    printf("You Sunk My "); // 
                    if (get_computer_ship_type() == 1) { // Carrier
                        printf("Carrier");
                    }
                    if (get_computer_ship_type() == 2) { // Cruiser
                        printf("Cruiser");
                    }
                    if (get_computer_ship_type() == 3) { // Destroyer
                        printf("Destroyer");
                    }
                    if (get_computer_ship_type() == 4) { // Frigate
                        printf("Frigate");
                    }
                    if (get_computer_ship_type() == 5) { // Corvette
                        printf("Corvette");
                    }
                    if (get_computer_ship_type() == 6) { // Submarine
                        printf("Submarine");
                    }
                    computers_ships_sunk += 1;
                }
                if (! is_game_over(humans_ships_sunk, computers_ships_sunk)) {

                    computer_fire(); // Automatically fire the computer's shot

                    if ((computer_hit() == 1)) {
                        hit_after_anim = 1;
                    }

                    if (computer_fire_success == 1) {
                        if ((computer_sunk_success = i_sunk()) == 1) {
                            move_terminal_cursor(50,terminal_i_sunk);
                            terminal_i_sunk += 1;
                            printf("I Sunk Your "); 
                            if (get_human_ship_type() == 1) { // Carrier
                                printf("Carrier");
                            }
                            if (get_human_ship_type() == 2) { // Cruiser
                                printf("Cruiser");
                            }
                            if (get_human_ship_type() == 3) { // Destroyer
                                printf("Destroyer");
                            }
                            if (get_human_ship_type() == 4) { // Frigate
                                printf("Frigate");
                            }
                            if (get_human_ship_type() == 5) { // Corvette
                                printf("Corvette");
                            }
                            if (get_human_ship_type() == 6) { // Submarine
                                printf("Submarine");
                            }
                            humans_ships_sunk += 1;
                        }
                        computer_move_increment(1,0); // Move the computer's next shot to be one right, next row if will go off the board
                    }
                }
            }
        }


        if ((computer_fire_animation() == 0) && (hit_after_anim == 1) && (can_play_sound == 1)) {
            hit_after_anim = 0;
            noise_playing_time = get_current_time();
            currently_playing_noise = note_1();
        }

        current_time = get_current_time();
        if (current_time >= last_flash_time + 200) {
            flash_cursor();
            last_flash_time = current_time;
        }

        if ((currently_cheating == 1) && (current_time >= cheating_time + 1000)) {
            stop_cheating();
            currently_cheating = 0;
        }

        // BLOCK FOR HUMAN SINKING SHIP (WORKS)
        if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 100) && (human_sunk_success == 1) && (humans_ships_sunk < 6) && (can_play_sound == 1)) {
            sound_off();
            currently_playing_noise = note_5();
            while (!(current_time >= noise_playing_time + 200)) {
                current_time = get_current_time();
            }
        }
        if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 200) && (human_sunk_success == 1) && (humans_ships_sunk < 6) && (can_play_sound == 1)) {
            sound_off();
            currently_playing_noise = note_6();
            while (!(current_time >= noise_playing_time + 300)) {
                current_time = get_current_time();
            }
        }
        if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 300) && (human_sunk_success == 1) && (humans_ships_sunk < 6) && (can_play_sound == 1)) {
            sound_off();
            currently_playing_noise = note_7();
            computer_sunk_success = 0; 
            while (!(current_time >= noise_playing_time + 400)) {
                current_time = get_current_time();
            }
        }

        // BLOCK FOR COMPUTER SINKING SHIP
        if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 100) && (computer_sunk_success == 1) && (computers_ships_sunk < 6) && (can_play_sound == 1)) {
            sound_off();
            currently_playing_noise = note_3();
            while (!(current_time >= noise_playing_time + 200)) {
                current_time = get_current_time();
            }
        }
        if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 200) && (computer_sunk_success == 1) && (computers_ships_sunk < 6) && (can_play_sound == 1)) {
            sound_off();
            currently_playing_noise = note_2();
            while (!(current_time >= noise_playing_time + 300)) {
                current_time = get_current_time();
            }
        }
        if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 300) && (computer_sunk_success == 1) && (computers_ships_sunk < 6) && (can_play_sound == 1)) {
            sound_off();
            currently_playing_noise = note_1();
            computer_sunk_success = 0; 
            while (!(current_time >= noise_playing_time + 400)) {
                current_time = get_current_time();
            }
        }
        
        if ((computer_sunk_success == 1) || (human_sunk_success == 1) && (can_play_sound == 1)) {
            // STOP SUNK SOUND AFTER 400ms
            if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 400)) {
                sound_off();
                currently_playing_noise = 0;
            }
        } else {
            // STOP SOUNDS AFTER 200ms
            if ((currently_playing_noise == 1) && (current_time >= noise_playing_time + 200) && (can_play_sound == 1)) {
                sound_off();
                currently_playing_noise = 0;
        }
        }
        } else {
        clear_to_end_of_line();
        move_terminal_cursor(10, 14);
        printf("GAME PAUSED");
        }
    }
    // We get here if the game is over.
}

void handle_game_over() {
    uint32_t current_time, noise_playing_time;

    game_over_lights();
    move_terminal_cursor(10, 14);
    printf_P(PSTR("GAME OVER. "));
    if (humans_ships_sunk == 6) {
        printf("Computer Wins!");
    }
    if (computers_ships_sunk == 6) {
        printf("Human Wins!");
    }
    move_terminal_cursor(10, 15);
    printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
    stagnant_ssd_value = 1;

    noise_playing_time = get_current_time();

    while (1) {
        current_time = get_current_time();

        // Human win sound sequence
        if (humans_ships_sunk == 6 && (can_play_sound == 1)) {
            if (current_time >= noise_playing_time + 100 && current_time < noise_playing_time + 200) {
                sound_off();
                currently_playing_noise = note_7();
            } else if (current_time >= noise_playing_time + 200 && current_time < noise_playing_time + 300) {
                sound_off();
                currently_playing_noise = note_6();
            } else if (current_time >= noise_playing_time + 300 && current_time < noise_playing_time + 400) {
                sound_off();
                currently_playing_noise = note_5();
            } else if (current_time >= noise_playing_time + 400 && current_time < noise_playing_time + 500) {
                sound_off();
                currently_playing_noise = note_4();
            } else if (current_time >= noise_playing_time + 500 && current_time < noise_playing_time + 600) {
                sound_off();
                currently_playing_noise = note_3();
            } else if (current_time >= noise_playing_time + 600 && current_time < noise_playing_time + 700) {
                sound_off();
                currently_playing_noise = note_2();
            } else if (current_time >= noise_playing_time + 700 && current_time < noise_playing_time + 800) {
                sound_off();
                currently_playing_noise = note_1();
            } else if (current_time >= noise_playing_time + 800) {
                sound_off();
                currently_playing_noise = 0;
            }
        }

        // Computer win sound sequence
        if (computers_ships_sunk == 6 && (can_play_sound == 1)) {
            if (current_time >= noise_playing_time + 100 && current_time < noise_playing_time + 200) {
                sound_off();
                currently_playing_noise = note_1();
            } else if (current_time >= noise_playing_time + 200 && current_time < noise_playing_time + 300) {
                sound_off();
                currently_playing_noise = note_2();
            } else if (current_time >= noise_playing_time + 300 && current_time < noise_playing_time + 400) {
                sound_off();
                currently_playing_noise = note_3();
            } else if (current_time >= noise_playing_time + 400 && current_time < noise_playing_time + 500) {
                sound_off();
                currently_playing_noise = note_4();
            } else if (current_time >= noise_playing_time + 500 && current_time < noise_playing_time + 600) {
                sound_off();
                currently_playing_noise = note_5();
            } else if (current_time >= noise_playing_time + 600 && current_time < noise_playing_time + 700) {
                sound_off();
                currently_playing_noise = note_6();
            } else if (current_time >= noise_playing_time + 700 && current_time < noise_playing_time + 800) {
                sound_off();
                currently_playing_noise = note_7();
            } else if (current_time >= noise_playing_time + 800) {
                sound_off();
                currently_playing_noise = 0;
            }
        }

        // Check for button presses or 's'/'S' input to start a new game
        btn = button_pushed();

        char serial_input = -1;
        if (serial_input_available()) {
            serial_input = fgetc(stdin);
        }

        if (btn == BUTTON0_PUSHED || btn == BUTTON1_PUSHED || btn == BUTTON2_PUSHED || btn == BUTTON3_PUSHED || serial_input == 's' || serial_input == 'S') {
            return;
        }
    }
}



