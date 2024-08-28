#ifndef JMOVEMENT
#define JMOVEMENT

//sets empty jewels new types and sets their downward motion
void set_falling(game_struct* mat);

//sets jewels (x,y) velocity vector
void set_jewel_motion(jewel* j1, float x_speed, float y_speed);

//sets jewel current position
void set_jewel_position(jewel* j1, vec2 new_current_position);

//swap types and powerups of 2 given jewels 
void swap_jewels_types(jewel* j1, jewel* j2);

/*sets jewel "j1" proper position to the neighbor slot in the direction "direction"
  and sets it to move to the slot with velocity "speed".
  sets jewel "j2" to the opposit direction.*/
void swap_jewels(jewel* j1, jewel* j2, dir_type direction, float speed);

/*updates position of specified jewel
  returns 0 if movement has ended
  otherwise, returns 1*/
int update_jewel(jewel *jewel);

/*updates all destroied jewels positions
  also updates their transparency (to make them vanish)
  returns 0 if jewel's movement has ended
  otherwise, returns 1*/
int update_destroied_jewels(game_struct *mat);

/*updates all jewels positions
  returns 0 if jewel's movement has ended
  otherwise, returns 1*/
int update_all_jewels(game_struct *mat);

#endif
