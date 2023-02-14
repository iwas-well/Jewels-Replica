#ifndef __JMOVEMENT__
#define __JMOVEMENT__

#include "structs.h"
#include "allegro_dependecies.h"
#include "libgame.h"

/*sets empty jewels new types and sets their downward motion*/
void set_falling(game_struct* mat);

void set_jewel_motion(jewel* j1, float x_speed, float y_speed);

void set_jewel_position(jewel* j1, vec2 new_current_position);

void swap_jewels_types(jewel* j1, jewel* j2);

/*swap jewels 'a' and 'b' types and set their velocity in opposit directions,
 * x_speed y_speed being the x velocity and y velocity of jewel 'a'*/
void swap_jewels(jewel* j1, jewel* j2, float x_speed, float y_speed);

/*updates position of specified jewel
 * returns 0 if movement has ended
 * otherwise, returns 1*/
int update_jewel(jewel *jewel);

/*updates all jewels positions
 * returns 0 if jewel's movement has ended
 * otherwise, returns 1*/
int update_all_jewels(game_struct *mat);

#endif
