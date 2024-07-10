/*
 * game.h
 *
 * Author: Jarrod Bennett, Cody Burnett
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */


#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

// Initialise the game by resetting the grid and beat
void initialise_game(void);

// flash the cursor
void flash_cursor(void);

// move the cursor in the x and/or y direction
void move_cursor(int8_t dx, int8_t dy);

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(int humans_ships_sunk, int computers_ships_sunk);

// fire at the location of a cursor, green for water and red for ship
uint8_t fire_cursor(void);

// fire at the location of the cursor as well as 8 tiles around it, green for water and red for ship
uint8_t cheating_b(void);

// fire at the whole row of the cursor, green for water and red for ship
uint8_t cheating_n(void);

// fire at the whole column of the cursor, green for water and red for ship
uint8_t cheating_m(void);

// computer automatically fire in sequential locations after fire_cursor(void)
uint8_t computer_fire(void);

// starts the computer placement in the top left corner, increments one space each fire
void computer_move_increment(int8_t cx, int8_t cy);

// checks if the firing location has already been fired upon and disallows firing if so
void invalid_move(void);

// checks if the player sunk a ship
uint8_t you_sunk(void);

// checks if the computer sunk a ship
uint8_t i_sunk(void);

// gets the human's type of ship
uint8_t get_human_ship_type(void);

// gets the computer's type of ship
uint8_t get_computer_ship_type(void);

// lights up the rest of the board if the game is over; dark orange for unhit ships, dark green for unhit water
void game_over_lights(void);

// starts the cheating method after 'c' / 'C' is pressed
uint8_t cheating(void);

// stops the cheating sequence after one second
void stop_cheating(void);

// checks if the human's fire hits a ship
uint8_t human_hit(void);

// checks if the computer's fire hits a ship
uint8_t computer_hit(void);

// animation in the human grid for when the computer fires
uint8_t computer_fire_animation(void);

uint8_t get_anim_count(void);

void set_anim_count(uint8_t val);

#define SEA 0
#define CARRIER 1
#define CRUISER 2
#define DESTROYER 3
#define FRIGATE 4
#define CORVETTE 5
#define SUBMARINE 6
#define SHIP_MASK 7
#define SHIP_END 8
#define HORIZONTAL 16

#endif
