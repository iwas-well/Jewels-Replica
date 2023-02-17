#ifndef __libgame__
#define __libgame__

#include <stdlib.h>

#include "structs.h"
#include "allegro_dependencies.h"
#include "jewels_movement.h"

//game configuration
#define SC_W        1000 //screen width
#define SC_H        750  //screen height
#define ROW_QT      8    //quantity of lines on the game matrix
#define COL_QT      8    //quantity of columns on the game matrix
#define JEWEL_SIZE  65   //lenght of jewel slot
#define VELOCITY    4    //jewel movement velocity
#define FIRST_SCORE_GOAL    250
#define NEW_LEVEL_TIMER     30 //num of iterations on new level state
#define SORTED_PER_FRAME    5  //num of swappings per frame on new level state
#define NEW_LEVEL_SLOW_DOWN 3  //amount of times new level state is slown by
#define FIRST_AVAILABLE_JEWELS  5 //number of jewels available in first level
#define MAX_AVAILABLE_JEWELS    6
#define WAIT_FRAMES             20  //number of frames spent on wait
                         
//game states
#define INPUT       0
#define JEWEL       1
#define DROP        2
#define SWAP        3
#define END_GAME    4
#define PAUSE       5
#define NEW_LEVEL   6
#define HELP_PAGE   7
#define WAIT        8

//status
#define NONE            0
#define DESTROY         1
#define DESTROY_STAR    2
#define DESTROY_SQUARE  3

//powerup
#define NONE        0
#define STAR        1
#define SQUARE      2
#define DIAMOND     3

vec2 get_rowcol(int x, int y, game_struct* mat);

void increase_score(game_struct* mat, int seq_size);

void increase_score_power(game_struct* mat, int powerup);

j_type get_new_type(int avail_jewels);

int test_possible_col_match(game_struct* mat, int row, int col);

int test_possible_row_match(game_struct* mat, int row, int col);

int test_end_game(game_struct* mat);

int register_user_input(ALLEGRO_EVENT* event, game_struct* mat);

int test_row(game_struct *mat, int row);

int test_col(game_struct *mat, int col);

int test_swap(game_struct *mat);

//*swap jewels 'a' and 'b' types and set their velocity in opposit directions,
// * x_speed y_speed being the x velocity and y velocity of jewel 'a'*/
void swap_jewels(jewel* j1, jewel* j2, float x_speed, float y_speed);

void sort_jewels(game_struct* mat,int swap_num);

jewel** allocate_jewel_matrix(int row, int col);

int initialize_jewel_structure(game_struct *mat);

int min(int a, int b);

#endif
