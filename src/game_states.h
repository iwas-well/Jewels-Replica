#ifndef JGAME_STATES
#define JGAME_STATES

#include "structs.h"

int jewel_state_logic(game_struct *mat);

int input_state_logic(game_struct *mat);

int swap_state_logic(game_struct *mat);

int update_sate_logic(game_struct *mat);

int new_level_state_logic(game_struct *mat);

int wait_state_logic(game_struct *mat);

int handle_game_state(game_struct *mat);

#endif
