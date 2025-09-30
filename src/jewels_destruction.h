#ifndef JDESTRUCT
#define JDESTRUCT

/*destroys jewels in given row and column
  (destroyer_type used in case a diamond is hit)*/
void star_destroyer(game_struct* mat, int row, int col, j_type destroyer_type);

/*destroys jewels around given row and column
  (destroyer_type used in case a diamond is hit)*/
void square_destroyer(game_struct* mat, int row, int col, j_type destroyer_type);

//destroys jewels of type "destroy type"
void diamond_destroyer(game_struct *mat, j_type destroy_type);

/*returns size of same type col sequence from up to bottom starting at
  jewel with given row,col*/
int get_col_sequence_size(game_struct* mat, int row, int col);

/*returns size of same type row sequence from left to right starting at
  jewel with given row, col*/
int get_row_sequence_size(game_struct* mat, int row, int col);

/*sets "status" attribute of jewels to be destroyed as "DESTROY",
  count same type sequences sizes and increases score accordingly
  and creates powerups*/
int set_to_destroy_matched_jewels(game_struct* mat);

/*Destroys jewel in given "row", "col" and increases the player score.
  Copies the jewel to the "mat->destroyed" jewel matrix,
  sets jewel type as empty and removes its status and power.
  If jewel had a power, destroys other jewels accordingly*/
void destroy_jewel_on_slot(game_struct* mat, int row, int col, j_type destroy_type);

//destroy all jewels with status set as "destroy"
void destroy_jewels(game_struct* mat);

//sets new "type" and "power" attribute of all jewels
void create_new_powers(game_struct* mat);

//sets new "type" and "power" attribute of jewel in given "row", "col"
void create_new_power_on_slot(game_struct* mat, int row, int col);

#endif
