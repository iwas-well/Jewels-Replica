#ifndef JRENDER_GAME
#define JRENDER_GAME

#include "structs.h"

void draw_jewel(game_struct* mat, jewel* j1);

void draw_ui(game_struct* mat);

int render_game_frame(game_struct* mat);

#endif
