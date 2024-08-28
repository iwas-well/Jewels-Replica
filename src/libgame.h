#ifndef JLIBGAME
#define JLIBGAME

#define SCORE_FILE_PATH "./resources/files/score_file.txt"

//game configuration
#define SC_W        1000 //screen width
#define SC_H        750  //screen height
#define ROW_QT      8    //quantity of lines on the game matrix
#define COL_QT      8    //quantity of columns on the game matrix
#define FPS         60.0
                         
#define MATRIX_SIZE         530     //matrix size in pixels
#define JEWEL_SIZE  ((MATRIX_SIZE/ROW_QT)-(15/ROW_QT))  //lenght of each jewel slot
#define JEWEL_SPRITE_SIZE   65      //lenght of jewel sprite in pixels

#define VELOCITY        240     //jewel movement velocity (pixels per second)
#define GRAVITY_ACCEL    12     //jewel vertical movement acceleration (pixels per second)
#define VELOCITY_PER_FRAME (float)(VELOCITY)/FPS
#define GRAVITY_ACCEL_PER_FRAME (float)(GRAVITY_ACCEL)/FPS

#define FIRST_SCORE_GOAL    250
#define NEW_LEVEL_TIMER     1.8 //time spent on new level transition animation (in seconds)
#define NEW_LEVEL_SLOW_DOWN 3   //amount of times new level state is slown by
#define SORTED_PER_FRAME    1   //num of swappings per frame on new level state
#define FIRST_AVAILABLE_JEWELS  5   //number of jewels available in first level
#define MAX_AVAILABLE_JEWELS    6   //number of maximum available jewels colors
#define WAIT_FRAMES             20  //number of frames spent on wait
                         
//game states
#define INPUT       0
#define JEWEL       1
#define UPDATE      2
#define SWAP        3
#define END_GAME    4
#define PAUSE       5
#define NEW_LEVEL   6
#define HELP_PAGE   7
#define WAIT        8

//status
#define DESTROY         1
#define DESTROY_STAR    2
#define DESTROY_SQUARE  3

//powerup
#define STAR        1
#define SQUARE      2
#define DIAMOND     3

#define NONE            0

/*given (x,y) screen position, returns vector (row,col) with of the corresponding
  slot position on the jewel matrix*/
vec2 get_rowcol(int x, int y, game_struct* mat);

//incerases game score according to given sequence size
void increase_score(game_struct* mat, int seq_size);

//incerases game score according to given powerup
void increase_score_power(game_struct* mat, int powerup);

//returns random jewel in the [0..avai_jewels] range
j_type get_new_type(int avail_jewels);

/*tests if there is any possible move left
  returns 0 if there is at least one move left
  returns 1 if game ended*/
int test_end_game(game_struct* mat);

/*register user input, setting number of selected jewels and 
  the jewels chosen to be swapped*/
int register_user_selection(ALLEGRO_EVENT* event, game_struct* mat);

/*set the "swap_direction" attribute as the direction from the first selected 
  jewel to the second*/
void set_swap_direction(game_struct *mat);

//tests if there is any match 3 in the given row
int test_row(game_struct *mat, int row);

//tests if there is any match 3 in the given col
int test_col(game_struct *mat, int col);

/*tests if swap should be allowed (if swap forms a matching sequence)
  returns 1 if sequence is formed by swapping
  returns 0 if no sequence is formed*/
int test_swap(game_struct *mat);

//swap random jewels 'swap_num' times
void sort_jewels(game_struct* mat,int swap_num);

jewel** allocate_jewel_matrix(int row, int col);

int initialize_jewel_structure(game_struct *mat);

int min(int a, int b);

//change state to new_state and records last_state game attribute
void change_state(game_struct *mat, int new_state);

//start in-game timer with an interval of "sec" seconds
void set_timer(game_struct *mat, float sec);

//set game frame rate to an interval of "sec" seconds
void set_frame_rate(game_struct *mat, float sec);

//set frame rate to 60 FPS 
void reset_frame_rate(game_struct *mat);

//stops in-game timer (stops generating timer events)
void stop_timer(game_struct *mat);

//restore music position and sets it to play
void continue_music(game_struct *mat);

//saves music position and stops it
void pause_music(game_struct *mat);

void toggle_pause_state(game_struct *mat, int state);

int load_best_score(game_struct *mat);

/*sets "score" attribute as the best score and saves it
  returns 0 if score was saved successfully
  returns 1 if score could not be saved*/
int save_best_score(game_struct *mat);

#endif
