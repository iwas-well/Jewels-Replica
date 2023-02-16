#ifndef __AL_DEPENDENCIES__
#define __AL_DEPENDENCIES__

#include <stdio.h>
#include <allegro5/color.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/display.h>
#include <allegro5/events.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#include "structs.h"
#include "libgame.h" 

void must_init(bool test, const char *description);

void deallocate_allegro_structures(game_struct* game_st);

int initialize_allegro_dependencies(game_struct* game_st);

#endif
