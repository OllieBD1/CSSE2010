/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "ledmatrix.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer2.h"

uint8_t human_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS];
uint8_t computer_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS];
uint8_t cursor_x, cursor_y; // cursor current position
uint8_t computer_x, computer_y; // computer current positon
uint8_t cursor_on;
uint8_t sunkcheck_x, sunkcheck_y; // checks if ship is sunk
uint32_t last_animation_time;
uint8_t anim_count;
uint8_t anim_flag;

// Initialise the game by resetting the grid and beat
void initialise_game(void) 
{
	// clear the splash screen art
	ledmatrix_clear();
	
	// see "Human Turn" feature for how ships are encoded
	// fill in the grid with the ships

	uint8_t initial_human_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS] =
		{{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {SEA,                  CARRIER|HORIZONTAL|SHIP_END,    CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL, CARRIER|HORIZONTAL, CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL|SHIP_END,    SEA                 },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {SEA,                  SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            SEA                 },
		 {DESTROYER|SHIP_END,   SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            FRIGATE|SHIP_END    },
		 {DESTROYER,            SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            FRIGATE             },
		 {DESTROYER|SHIP_END,   SEA,                            CRUISER|HORIZONTAL|SHIP_END,    CRUISER|HORIZONTAL, CRUISER|HORIZONTAL, CRUISER|HORIZONTAL|SHIP_END,    SEA,                            FRIGATE|SHIP_END    },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 }};
	uint8_t initial_computer_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS] =
		{{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {DESTROYER|SHIP_END,   SEA,                            CRUISER|HORIZONTAL|SHIP_END,    CRUISER|HORIZONTAL, CRUISER|HORIZONTAL, CRUISER|HORIZONTAL|SHIP_END,    SEA,                            FRIGATE|SHIP_END    },
		 {DESTROYER,            SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            FRIGATE             },
		 {DESTROYER|SHIP_END,   SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            FRIGATE|SHIP_END    },
		 {SEA,                  SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            SEA                 },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {SEA,                  CARRIER|HORIZONTAL|SHIP_END,    CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL, CARRIER|HORIZONTAL, CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL|SHIP_END,    SEA                 },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 }};
	for (uint8_t i=0; i<GRID_NUM_COLUMNS; i++)
	{
		for (uint8_t j=0; j<GRID_NUM_COLUMNS; j++)
		{
			human_grid[j][i] = initial_human_grid[j][i];
			computer_grid[j][i] = initial_computer_grid[j][i];
			if (human_grid[j][i] & SHIP_MASK)
			{
				ledmatrix_draw_pixel_in_human_grid(i, j, COLOUR_ORANGE);
			}
		}
	}
	cursor_x = 3;
	cursor_y = 3;
	cursor_on = 1;
	computer_x = 0;
	computer_y = 7;
}

void flash_cursor(void)
{
	cursor_on = 1-cursor_on;
	if (cursor_on)
	{	
	// (1 << 5) is hit indicator -> if it is hit
	if (computer_grid[cursor_y][cursor_x] & (1 << 5)) { 
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_DARK_YELLOW); // hit tiles flash dark yellow
	} else {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_YELLOW); // non hit tiles flash yellow
	}
	// (1 << 5) is hit indicator, (1 << 0 | 1 << 1 | 1 << 2) is ship indicator -> else if is hit and is a ship
	} else if (computer_grid[cursor_y][cursor_x] & (1 << 5) && (computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2))) {
		if (computer_grid[cursor_y][cursor_x] & (1 << 6)) { // (1 << 6) is sunk indicator 
			ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_DARK_RED); // if sunk, dark red
		} else {
			ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_RED); // if not sunk,
		}
	} 
	// (1 << 5) is hit indicator
	else if (computer_grid[cursor_y][cursor_x] & (1 << 5)) 
	{
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_GREEN); // hit and is anything else (not a ship)
	}
	else if (computer_grid[cursor_y][cursor_x] & (1 << 7)) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_ORANGE);
	} else {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_BLACK); // is not hit
	}
}

// moves the position of the cursor by (dx, dy) such that if the cursor
// started at (cursor_x, cursor_y) then after this function is called,
// it should end at ( (cursor_x + dx) % WIDTH, (cursor_y + dy) % HEIGHT)
// the cursor should be displayed after it is moved as well
void move_cursor(int8_t dx, int8_t dy) {
	//   1: remove the display of the cursor at the current location
	//  		(and replace it with whatever piece is at that location) **
	ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_BLACK);
	if (computer_grid[cursor_y][cursor_x] & (1 << 6)) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_DARK_RED); // Sunk
	} else if (computer_grid[cursor_y][cursor_x] & (1 << 5) && (computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2))) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_RED); // Hit and is ship
	} else if (computer_grid[cursor_y][cursor_x] & (1 << 5)) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_GREEN); // Hit and is anything else (not a ship)
	} else if (computer_grid[cursor_y][cursor_x] & (1 << 7)) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_ORANGE);
	}

	cursor_on = 0;

	cursor_x += dx; // Update the positional knowledge of the cursor
	cursor_y += dy;

	if (cursor_x == 255) { // Considers if cursor_x moves left off the board (Note. 255 is 11111111 (-1) in unsigned)
		cursor_x = 7;
	} else if (cursor_x == 8) { // Considers if cursor_x moves right off the board
		cursor_x = 0;
	}

	if (cursor_y == 255) { // Considers if cursor_y moves down off the board
		cursor_y = 7;
	} else if (cursor_y == 8) { // Considers if cursor_y moves up off the board
		cursor_y = 0;
	}
	
}

void computer_move_increment(int8_t cx, int8_t cy) {
	// Check if the location under the cursor is a ship or a blank
	if (computer_grid[cursor_y][cursor_x] & (1 << 5)) {
	}
    else 
	if (human_grid[computer_y][computer_x] & (1 << 5) && (human_grid[computer_y][computer_x] & (1 << 0 | 1 << 1 | 1 << 2))) {
		ledmatrix_draw_pixel_in_human_grid(computer_x, computer_y, COLOUR_RED); // Hit and is ship
	} 
	else if (human_grid[computer_y][computer_x] & (1 << 5)) {
		ledmatrix_draw_pixel_in_human_grid(computer_x, computer_y, COLOUR_GREEN); // Hit and is anything else (not a ship)
	}

	if (computer_x == 7) { // Considers if computer_x moves right off the board 
		computer_x = 0; // Moves computer's location to LHS of the board
		computer_y -= 1; // Increments the computers firing location one row down
	}
	else {
		computer_x += cx; // Update the positional knowledge of the computer's firing location
	}
}

int sillyhumanretries = 0;

uint8_t fire_cursor(void) 
{
	// Check if the location under the cursor is a ship or a blank
	if (computer_grid[cursor_y][cursor_x] & (1 << 5)) { // If it has been hit, move through invalid sequence and disallow fire	
		if (sillyhumanretries == 0) {
			move_terminal_cursor(10,0);
        	printf("Invalid Move, Please Try Again. ");
		}
		else if (sillyhumanretries == 1) {
			clear_to_end_of_line();
			move_terminal_cursor(10,0);
			printf ("Invalid again. You Already Shot There... ");
		}
		else if (sillyhumanretries >= 2) {
			clear_to_end_of_line();
			move_terminal_cursor(10,0);
			printf("Invalid Again. Are You Trying To Be Funny? ");
		}
		sillyhumanretries += 1; // Increment the number of retries
		return 0;
    }
    else { // If it has not been hit, fire, update ships to red, update else to green 
	{
		move_terminal_cursor(0,0);
		sillyhumanretries = 0;
		clear_to_end_of_line();
		computer_grid[cursor_y][cursor_x] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				if (!cursor_on) { 	// If location under the cursor is a ship, update from black to red
					ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_RED); 
		}
	}
	else if (!cursor_on) { 	// If location under the cursor is not a ship, update from black to green
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_GREEN);
	}
	return 1;
	}
	}
}

int sillyhumancheatries = 0; 
int b_cheating_times = 0;
int m_cheating_times = 0;
int n_cheating_times = 0;

// firing at the cursor location and the 8 tiles around it
uint8_t cheating_b(void) 
{
    if (b_cheating_times > 0) {
        move_terminal_cursor(10, 0);
        printf("You've Already Used Cheat 'B'");
        return 0;
    }
    // Check if the location under the cursor is a ship or a blank
    if (computer_grid[cursor_y][cursor_x] & (1 << 5)) { // If it has been hit, move through invalid sequence and disallow fire    
        if (sillyhumanretries == 0) {
            move_terminal_cursor(10, 0);
            printf("Invalid Move, Please Try Again.");
        } else if (sillyhumanretries == 1) {
            clear_to_end_of_line();
            move_terminal_cursor(10, 0);
            printf("Invalid again. You Already Shot There...");
        } else if (sillyhumanretries >= 2) {
            clear_to_end_of_line();
            move_terminal_cursor(10, 0);
            printf("Invalid Again. Are You Trying To Be Funny?");
        }
        sillyhumanretries += 1; // Increment the number of retries
        return 0;
    }

    else { // If it has not been hit, fire, update ships to red, update else to green 
	{
		move_terminal_cursor(0,0);
		clear_to_end_of_line();
		// On the cursor
		computer_grid[cursor_y][cursor_x] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_GREEN);
			}
		// Below the cursor
		computer_grid[cursor_y - 1][cursor_x] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y - 1][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y - 1, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y - 1, COLOUR_GREEN);
			}
		// Bottom left of the cursor
		computer_grid[cursor_y - 1][cursor_x - 1] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y - 1][cursor_x - 1] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x - 1, cursor_y - 1, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x - 1, cursor_y - 1, COLOUR_GREEN);
			}
		// Left of the cursor
		computer_grid[cursor_y][cursor_x - 1] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y][cursor_x - 1] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x - 1, cursor_y, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x - 1, cursor_y, COLOUR_GREEN);
			}
		// Top left of the cursor
		computer_grid[cursor_y + 1][cursor_x - 1] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y + 1][cursor_x - 1] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x - 1, cursor_y + 1, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x - 1, cursor_y + 1, COLOUR_GREEN);
			}
		// Above the cursor
		computer_grid[cursor_y + 1][cursor_x] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y + 1][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y + 1, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y + 1, COLOUR_GREEN);
			}
		// Top right of the cursor
		computer_grid[cursor_y + 1][cursor_x + 1] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y + 1][cursor_x + 1] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				ledmatrix_draw_pixel_in_computer_grid(cursor_x + 1, cursor_y + 1, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x + 1, cursor_y + 1, COLOUR_GREEN);
			}
		// Right of the cursor
		computer_grid[cursor_y][cursor_x + 1] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y][cursor_x + 1] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x + 1, cursor_y, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x + 1, cursor_y, COLOUR_GREEN);
			}
		// Bottom right of the cursor
		computer_grid[cursor_y - 1][cursor_x + 1] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
			if (computer_grid[cursor_y - 1][cursor_x + 1] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x + 1, cursor_y - 1, COLOUR_RED); 
			} else {
				ledmatrix_draw_pixel_in_computer_grid(cursor_x + 1, cursor_y - 1, COLOUR_GREEN);
			}
	b_cheating_times += 1;
	return 1;
	}
	}
	}

// firing at every tile in the row of the cursor location
uint8_t cheating_n(void)
{
    if (n_cheating_times > 0) {
        move_terminal_cursor(10, 0);
        printf("You've Already Used Cheat 'N'");
        return 0;
    }
    // Check if the location under the cursor is a ship or a blank
    if (computer_grid[cursor_y][cursor_x] & (1 << 5)) { // If it has been hit, move through invalid sequence and disallow fire    
        if (sillyhumanretries == 0) {
            move_terminal_cursor(10, 0);
            printf("Invalid Move, Please Try Again.");
        } else if (sillyhumanretries == 1) {
            clear_to_end_of_line();
            move_terminal_cursor(10, 0);
            printf("Invalid again. You Already Shot There...");
        } else if (sillyhumanretries >= 2) {
            clear_to_end_of_line();
            move_terminal_cursor(10, 0);
            printf("Invalid Again. Are You Trying To Be Funny?");
        }
        sillyhumanretries += 1; // Increment the number of retries
        return 0;
    } else { // If it has not been hit, fire, update ships to red, update else to green
        move_terminal_cursor(0, 0);
        sillyhumanretries = 0;
        clear_to_end_of_line();
        for (uint8_t i = 0; i < 8; i += 1) {
            computer_grid[cursor_y][i] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
            if (computer_grid[cursor_y][i] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(i, cursor_y, COLOUR_RED);
            } else { 
				// If location under the cursor is not a ship, update from black to green
                ledmatrix_draw_pixel_in_computer_grid(i, cursor_y, COLOUR_GREEN);
            }
        }
		n_cheating_times += 1;
        return 1;
    }
}

// firing at every tile in the column of the cursor location
uint8_t cheating_m(void)
{
    if (m_cheating_times > 0) {
        move_terminal_cursor(10, 0);
        printf("You've Already Used Cheat 'M'");
        return 0;
    }
    // Check if the location under the cursor is a ship or a blank
    if (computer_grid[cursor_y][cursor_x] & (1 << 5)) { // If it has been hit, move through invalid sequence and disallow fire    
        if (sillyhumanretries == 0) {
            move_terminal_cursor(10, 0);
            printf("Invalid Move, Please Try Again.");
        } else if (sillyhumanretries == 1) {
            clear_to_end_of_line();
            move_terminal_cursor(10, 0);
            printf("Invalid again. You Already Shot There...");
        } else if (sillyhumanretries >= 2) {
            clear_to_end_of_line();
            move_terminal_cursor(10, 0);
            printf("Invalid Again. Are You Trying To Be Funny?");
        }
        sillyhumanretries += 1; // Increment the number of retries
        return 0;
    } else { // If it has not been hit, fire, update ships to red, update else to green
        move_terminal_cursor(0, 0);
        sillyhumanretries = 0;
        clear_to_end_of_line();
        for (uint8_t i = 0; i < 8; i += 1) {
            computer_grid[i][cursor_x] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
            if (computer_grid[i][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2)) { // Bit mask; if 0 in water, if 1 not in water
				// If location under the cursor is a ship, update from black to red
				ledmatrix_draw_pixel_in_computer_grid(cursor_x, i, COLOUR_RED);
            } else { 
				// If location under the cursor is not a ship, update from black to green
                ledmatrix_draw_pixel_in_computer_grid(cursor_x, i, COLOUR_GREEN);
            }
        }
		m_cheating_times += 1;
        return 1;
    }
}

uint8_t human_hit(void) {
	// If location is hit (invalid move), do nothing.
	if (sillyhumanretries >= 1) {
		return 0;
	}
	// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
	else if ((computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2))) {
		return 1;
	}
		return 0;
}

uint8_t computer_hit(void) {
	// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
	if ((human_grid[computer_y][computer_x] & (1 << 0 | 1 << 1 | 1 << 2))) {
		return 1;
	}
	return 0;
}

uint8_t computer_fire(void) {
	if (sillyhumanretries >= 1) {
		return 0;
	} else {
		// Check if the location under the cursor is a ship or a blank
		human_grid[computer_y][computer_x] |= (1 << 5); // Bit 5 is hit indicator, setting 5 to 1 when a ship or water is hit
		if (human_grid[computer_y][computer_x] & (1 << 0 | 1 << 1 | 1 << 2)) {
		// Bit mask; if 0 in water, if 1 not in water
		ledmatrix_draw_pixel_in_human_grid(computer_x, computer_y, COLOUR_RED); 
		} // If location under the computer_fire(void) is a ship, update from black to red
		else {
		ledmatrix_draw_pixel_in_human_grid(computer_x, computer_y, COLOUR_GREEN); 
		} // If location under the computer_fire(void) is not a ship, update from black to greens
		return 1;
	}
}

uint8_t you_sunk(void)
	// > if HIT
	// >> if SHIP
	// >>> if HORIZONTAL
	// >>>>> check END, if not END, move RIGHT, check each time if HIT in x direction until you hit END
	// >>>>> check END, if not END, move LEFT, check each time if HIT in the x direction until you hit END ** NEED TO SOMEHOW DETERMINE WHAT SHIP IT IS.
	// >>> else VERTICAL
	// >>>>> check END, if not END, move UP, check each time if HIT in y direction until you hit END
	// >>>>> check END, if not END, move DOWN, check each time if HIT in the y direction until you hit END ** NEED TO SOMEHOW DETERMINE WHAT SHIP IT IS.
	// >> Else (!Ship)
	// >> return 0; (Not sunk)
	// return 1;
{
	// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
	uint8_t computer_type = (computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2));
	uint8_t ship_length;
	uint8_t segments_hit = 0; 

	if (computer_grid[cursor_y][cursor_x] & (1 << 6)) {
		return 0;
	} 
	if (computer_type == 0) { // Water
		return 0;
	}
	if (computer_type == 1) { // Carrier
		ship_length = 6; 
	}
	if (computer_type == 2) { // Cruiser
		ship_length = 4;
	}
	if (computer_type == 3) { // Destroyer
		ship_length = 3;
	}
	if (computer_type == 4) { // Frigate
		ship_length = 3;
	}
	if (computer_type == 5) { // Corvette
		ship_length = 2;
	}
	if (computer_type == 6) { // Submarine
		ship_length = 2;
	}

	if (computer_grid[cursor_y][cursor_x] & (1 << 4)) { // If horizontal
		for (uint8_t i = 0; i < 8; i += 1) {
			uint8_t typeofgrid = computer_grid[cursor_y][i] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
			if (typeofgrid == computer_type) {
				if (computer_grid[cursor_y][i] & (1 << 5)) { // If hit
					segments_hit += 1;
				} 
			}	
		}
	} else { // If vertical
		for (uint8_t i = 0; i < 8; i += 1) {
			uint8_t typeofgrid = computer_grid[i][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
			if (typeofgrid == computer_type) {
				if (computer_grid[i][cursor_x] & (1 << 5)) { // If hit 
					segments_hit += 1;
				} 
			}
		} 
	}
	if (segments_hit == ship_length) {
		computer_grid[cursor_y][cursor_x] |= (1 << 6); // Bit 6 is sunk indicator
			if (computer_grid[cursor_y][cursor_x] & (1 << 4)) { // If horizontal
				for (uint8_t i = 0; i < 8; i += 1) {
					uint8_t typeofgrid = computer_grid[cursor_y][i] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
					if (typeofgrid == computer_type) {
						if (computer_grid[cursor_y][i] & (1 << 5)) { // If hit
							ledmatrix_draw_pixel_in_computer_grid(i, cursor_y, COLOUR_DARK_RED);
							computer_grid[cursor_y][i] |= (1 << 6);
					segments_hit += 1;
						}
					} 
				}	
			} else { // If vertical
				for (uint8_t i = 0; i < 8; i += 1) {
					uint8_t typeofgrid = computer_grid[i][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
					if (typeofgrid == computer_type) {
						if (computer_grid[i][cursor_x] & (1 << 5)) { // If hit 
							ledmatrix_draw_pixel_in_computer_grid(cursor_x, i, COLOUR_DARK_RED);
							computer_grid[i][cursor_x] |= (1 << 6);
					segments_hit += 1;
				} 
			}
		}
	}
	return 1;

	} else {
		return 0;
	}
}

uint8_t i_sunk(void)
	// > if HIT
	// >> if SHIP
	// >>> if HORIZONTAL
	// >>>>> check END, if not END, move RIGHT, check each time if HIT in x direction until you hit END
	// >>>>> check END, if not END, move LEFT, check each time if HIT in the x direction until you hit END 
	// >>> else VERTICAL
	// >>>>> check END, if not END, move UP, check each time if HIT in y direction until you hit END
	// >>>>> check END, if not END, move DOWN, check each time if HIT in the y direction until you hit END 
	// >> Else (!Ship)
	// >> return 0; (Not sunk)
	// return 1;
{
	// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
	uint8_t human_type = (human_grid[computer_y][computer_x] & (1 << 0 | 1 << 1 | 1 << 2));
	uint8_t ship_length;
	uint8_t segments_hit = 0; 
	if (human_type == 0) { // Water
		return 0;
	}
	if (human_type == 1) { // Carrier
		ship_length = 6; 
	}
	if (human_type == 2) { // Cruiser
		ship_length = 4;
	}
	if (human_type == 3) { // Destroyer
		ship_length = 3;
	}
	if (human_type == 4) { // Frigate
		ship_length = 3;
	}
	if (human_type == 5) { // Corvette
		ship_length = 2;
	}
	if (human_type == 6) { // Submarine
		ship_length = 2;
	}

	if (human_grid[computer_y][computer_x] & (1 << 4)) { // If horizontal
		for (uint8_t i = 0; i < 8; i += 1) {
			uint8_t typeofgrid = human_grid[computer_y][i] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
			if (typeofgrid == human_type) {
				if (human_grid[computer_y][i] & (1 << 5)) { // If hit
					segments_hit += 1;
				} 
			}	
		}
	} else { // If vertical
		for (uint8_t i = 0; i < 8; i += 1) {
			uint8_t typeofgrid = human_grid[i][computer_x] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
			if (typeofgrid == human_type) {
				if (human_grid[i][computer_x] & (1 << 5)) { // If hit 
					segments_hit += 1;
				} 
			}
		}
	}
	if (segments_hit == ship_length) {
		human_grid[computer_y][computer_x] |= (1 << 6); // (1 << 6) is sunk indicator
			if (human_grid[computer_y][computer_x] & (1 << 4)) { // If horizontal
				for (uint8_t i = 0; i < 8; i += 1) {
					uint8_t typeofgrid = human_grid[computer_y][i] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
					if (typeofgrid == human_type) {
						if (human_grid[computer_y][i] & (1 << 5)) { // If hit
							ledmatrix_draw_pixel_in_human_grid(i, computer_y, COLOUR_DARK_RED);
							human_grid[computer_y][i] |= (1 << 6);
					segments_hit += 1;
						}
					} 
				}	
			} else { // If vertical
				for (uint8_t i = 0; i < 8; i += 1) {
					uint8_t typeofgrid = human_grid[i][computer_x] & (1 << 0 | 1 << 1 | 1 << 2); // Check ship type
					if (typeofgrid == human_type) {
						if (human_grid[i][computer_x] & (1 << 5)) { // If hit 
							ledmatrix_draw_pixel_in_human_grid(computer_x, i, COLOUR_DARK_RED);
							human_grid[i][computer_x] |= (1 << 6);
					segments_hit += 1;
				} 
			}
		}
	}
	return 1;

	} else {
		return 0;
	}
}

uint8_t get_computer_ship_type(void) {
	// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
	uint8_t computer_type = (computer_grid[cursor_y][cursor_x] & (1 << 0 | 1 << 1 | 1 << 2));
	return computer_type;
}

uint8_t get_human_ship_type(void) {
	// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
	uint8_t human_type = (human_grid[computer_y][computer_x] & (1 << 0 | 1 << 1 | 1 << 2));
	return human_type;
}

void game_over_lights(void) {
	for (uint8_t y_check = 0; y_check < 8; y_check += 1) {
		for (uint8_t x_check = 0; x_check < 8; x_check += 1) {
			// (1 << 5) is hit indicator
			if (computer_grid[y_check][x_check] & (1 << 5)) {
			}
			// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
			else if (computer_grid[y_check][x_check] & (1 << 0 | 1 << 1 | 1 << 2)) {
				ledmatrix_draw_pixel_in_computer_grid(x_check, y_check, COLOUR_DARK_ORANGE);
			}
			else {
				ledmatrix_draw_pixel_in_computer_grid(x_check, y_check, COLOUR_DARK_GREEN);
			}
			// (1 << 5) is hit indicator
			if (human_grid[y_check][x_check] & (1 << 5)) {
			}
			// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
			else if (human_grid[y_check][x_check] & (1 << 0 | 1 << 1 | 1 << 2)) {
				ledmatrix_draw_pixel_in_human_grid(x_check, y_check, COLOUR_DARK_ORANGE);
			}
			else {
				ledmatrix_draw_pixel_in_human_grid(x_check, y_check, COLOUR_DARK_GREEN);
			}
		}
	}
}

uint8_t cheating(void) {
	for (uint8_t y_check = 0; y_check < 8; y_check += 1) {
		for (uint8_t x_check = 0; x_check < 8; x_check += 1) {
			// (1 << 5) is hit indicator, (1 << 6) is sink indicator
			if ((computer_grid[y_check][x_check] & (1 << 5)) || (computer_grid[y_check][x_check] & (1 << 6))) {
			}
			// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
			else if (computer_grid[y_check][x_check] & (1 << 0 | 1 << 1 | 1 << 2)) {
				// (1 << 7) is cheating indicator
				computer_grid[y_check][x_check] |= (1 << 7);
				ledmatrix_draw_pixel_in_computer_grid(x_check, y_check, COLOUR_ORANGE);
			}
		}
	}
	return 1;
}

	void stop_cheating(void) {
		for (uint8_t y_check = 0; y_check < 8; y_check += 1) {
			for (uint8_t x_check = 0; x_check < 8; x_check += 1) {
				// (1 << 5) is hit indicator, (1 << 6) is sink indicator
				if ((computer_grid[y_check][x_check] & (1 << 5)) || (computer_grid[y_check][x_check] & (1 << 6))) {
				}
				// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
				else if (computer_grid[y_check][x_check] & (1 << 0 | 1 << 1 | 1 << 2)) {
				// (1 << 7) is cheating indicator
				computer_grid[y_check][x_check] &= ~(1 << 7);
				ledmatrix_draw_pixel_in_computer_grid(x_check, y_check, COLOUR_BLACK);
			}
			}
			}
	}

void set_anim_count(uint8_t val) {
	anim_count = val;
}

uint8_t get_anim_count(void) {
	return anim_count;
}

void paint_under_colour(uint8_t temp_x, uint8_t temp_y)
{
	if (human_grid[temp_y][temp_x] & (1 << 6)) {
		ledmatrix_draw_pixel_in_human_grid(temp_x, temp_y, COLOUR_DARK_RED); // Sunk
		// (1 << 0 | 1 << 1 | 1 << 2) is ship indicator
		} else if (human_grid[temp_y][temp_x] & (1 << 5) && (human_grid[temp_y][temp_x] & (1 << 0 | 1 << 1 | 1 << 2))) {
		ledmatrix_draw_pixel_in_human_grid(temp_x, temp_y, COLOUR_RED); // Hit and is ship
			} else if (human_grid[temp_y][temp_x] & (1 << 5)) {
		ledmatrix_draw_pixel_in_human_grid(temp_x, temp_y, COLOUR_GREEN); // Hit and is anything else (not a ship)
			} else if (human_grid[temp_y][temp_x] & (1 << 7)) {
		ledmatrix_draw_pixel_in_human_grid(temp_x, temp_y, COLOUR_ORANGE);
			}
}

uint8_t computer_fire_animation(void) { // attempting to sort out firing animation

	uint32_t animation_time_game;

	animation_time_game = get_current_time();

	if (animation_time_game >= last_animation_time + 100) {
		uint8_t temp_x = computer_x;
		uint8_t temp_y = computer_y;
		if (temp_x == 0) {
			temp_x = 7;
			temp_y += 1;
		} else {
			temp_x -= 1;
		}
		if (anim_count) {
			if (anim_flag) {
				ledmatrix_draw_pixel_in_human_grid(temp_x, temp_y, COLOUR_YELLOW);
			} else {
				ledmatrix_draw_pixel_in_human_grid(temp_x, temp_y, COLOUR_BLACK);
			}
			anim_count -= 1;
			if (!anim_count) {
				paint_under_colour(temp_x, temp_y);
			}
		} else {
			paint_under_colour(temp_x, temp_y);
		}
		last_animation_time = animation_time_game;
		anim_flag = !anim_flag;
	}

	if (anim_count > 0) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t is_game_over(int humans_ships_sunk, int computers_ships_sunk)
{
	if (humans_ships_sunk == 6 || computers_ships_sunk == 6) {
		return 1;
	}

	return 0;
}