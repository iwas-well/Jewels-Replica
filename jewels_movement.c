#include "jewels_movement.h"

/*sets empty jewels new types and sets their downward motion*/
void set_falling(game_struct* mat)
{
    int i, j;
    j_type aux;

    for (i = ROW_QT-1; i >= 0; i--){
        for (j = 0; j < COL_QT; j++)
        {
            int above_last_row = (i < (ROW_QT-1));
            if (above_last_row)
                mat->jewels[i][j].lower = mat->jewels[i + 1][j].lower;
            else
                mat->jewels[i][j].lower = 0;

            int new_jewel_row = (i - mat->jewels[i][j].lower);
            while ((new_jewel_row >= 0) && (mat->jewels[new_jewel_row][j].type == EMPTY)){
                mat->jewels[i][j].lower ++;
                new_jewel_row = (i - mat->jewels[i][j].lower);
            }

            int above_should_drop = mat->jewels[i][j].lower;
            if (above_should_drop) {

                mat->jewels[i][j].vel.x = 0;
                mat->jewels[i][j].vel.y = VELOCITY;

                if (new_jewel_row >= 0){
                    mat->jewels[i][j].type = mat->jewels[new_jewel_row][j].type;
                    mat->jewels[i][j].power = mat->jewels[new_jewel_row][j].power;

                    mat->jewels[new_jewel_row][j].type  = EMPTY;
                    mat->jewels[new_jewel_row][j].power = NONE;

                    mat->jewels[i][j].current.y = mat->jewels[new_jewel_row][j].proper.y;
                }
                else {
                    j_type new_jewel = get_new_type(mat->available_jewels);
                    mat->jewels[i][j].type = new_jewel;

                    int new_jewel_height = mat->jewels[i][j].proper.y - (mat->jewels[i][j].lower * JEWEL_SIZE);
                    mat->jewels[i][j].current.y = new_jewel_height; 
                }

            }
        }
    }

    int last_row = (ROW_QT-1); 
    for (int i = last_row; i >= 0; i--) 
        for (j = 0; j < COL_QT; j++)
            mat->jewels[i][j].lower = 0;
}

void set_jewel_motion(jewel* j1, float x_speed, float y_speed)
{
    j1->vel.x = x_speed;
    j1->vel.y = y_speed;
}

void set_jewel_position(jewel* j1, vec2 new_current_position)
{
    j1->current.x = new_current_position.x;
    j1->current.y = new_current_position.y;
}

void swap_jewels_types(jewel* j1, jewel* j2)
{
    //swap types
    j_type aux_type;
    aux_type = j1->type;
    j1->type = j2->type; 
    j2->type = aux_type; 

    //swap power
    j_type aux_power;
    aux_power = j1->power;
    j1->power = j2->power; 
    j2->power = aux_power; 
}

/*swap jewels 'a' and 'b' types and set their velocity in opposit directions,
 * x_speed y_speed being the x velocity and y velocity of jewel 'a'*/
void swap_jewels(jewel* j1, jewel* j2, float x_speed, float y_speed)
{
    set_jewel_motion(j1, x_speed, y_speed);
    set_jewel_motion(j2, -x_speed, -y_speed);

    vec2 aux_position = j1->proper;
    set_jewel_position(j1, j2->proper);
    set_jewel_position(j2, aux_position);
    swap_jewels_types(j1, j2);
}

/*updates position of specified jewel
 * returns 0 if movement has ended
 * otherwise, returns 1*/
int update_jewel(jewel *jewel)
{
    int moving = 0;

    if (abs(jewel->current.x - jewel->proper.x) > VELOCITY ||
        abs(jewel->current.y - jewel->proper.y) > VELOCITY)
    {
        moving = 1;
        jewel->current.x += jewel->vel.x;
        jewel->current.y += jewel->vel.y;
    }
    else
    {
        jewel->current.x = jewel->proper.x;
        jewel->current.y = jewel->proper.y;
    }
    return moving;
}

/*updates all jewels positions
 * returns 0 if jewel's movement has ended
 * otherwise, returns 1*/
int update_all_jewels(game_struct *mat)
{
    int moving = 0;

    for (int row = 0; row < ROW_QT; row ++)
        for (int col = 0; col < COL_QT; col ++)
            moving = ( update_jewel(&(mat->jewels[row][col])) || moving );

    return moving;
}
