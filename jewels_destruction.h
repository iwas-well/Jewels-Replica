#include "structs.h"
#include "allegro_dependencies.h"
#include "libgame.h"

void star_destroyer(game_struct* mat, int row, int col, int destroyer_type);

void diamond_destroyer(game_struct *mat, j_type type);

void square_destroyer(game_struct* mat, int row, int col, int destroyer_type);

int col_sequence_size(game_struct* mat, int row, int col);

//returns size of row sequence from left to right starting at jewel
int row_sequence_size(game_struct* mat, int row, int col);

void destroy_jewels(game_struct* mat);

void destroy_jewel(game_struct* mat, int row, int col, int destroyer_type);

void create_new_powers(game_struct* mat);

int set_to_destroy_matched_jewels(game_struct* mat);
