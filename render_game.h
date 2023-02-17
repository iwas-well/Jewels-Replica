#ifndef __RENDER_GAME__
#define __RENDER_GAME__

#include "structs.h"
#include "allegro_dependencies.h"
#include "libgame.h"

void draw_jewel(game_struct* mat, jewel* j1);

void draw_ui(game_struct* mat);

int render_game_frame(game_struct* mat);

#endif
