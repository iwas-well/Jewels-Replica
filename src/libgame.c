#include <stdlib.h>
#include <stdio.h>

#include "structs.h"
#include "allegro_dependencies.h"
#include "jewels_movement.h"
#include "libgame.h"
#include "jewels_movement.h"

//given (x,y) screen position, returns vector (row,col) with of the corresponding
//slot position on the jewel matrix
vec2 get_rowcol(int x, int y, game_struct* mat)
{
    vec2 slot;
    slot.row = (y - mat->pos.y)/JEWEL_SIZE; 
    slot.col = (x - mat->pos.x)/JEWEL_SIZE;
    return slot;
}

//incerases game score according to given sequence size
void increase_score(game_struct* mat, int seq_size)
{
    if (seq_size == 3)
        mat->score += 10;
    else if (seq_size == 4)
        mat->score += 20;
    else if (seq_size >= 5)
        mat->score += 30;
}

//incerases game score according to given powerup
void increase_score_power(game_struct* mat, int powerup)
{
    if (powerup == SQUARE)
        mat->score += 50;
    else if (powerup == STAR)
        mat->score += 60;
    else if (powerup == DIAMOND)
        mat->score += 100;
}

//returns random jewel in the [0..avai_jewels] range
j_type get_new_type(int avail_jewels){
    return (rand()%avail_jewels);
}

//tests if jewel in given (row, col) is inside a jewel sequence
static int test_jewel_mid_sequence(game_struct* mat, int row, int col)
{
    //tests if jewel is in a column sequence
    if ((row +1 < ROW_QT) && (row - 1 >= 0))
        if ((mat->jewels[row][col].type == mat->jewels[row+1][col].type) &&
                (mat->jewels[row][col].type == mat->jewels[row-1][col].type))
            return 1;
    if (row +2 < ROW_QT)
        if ((mat->jewels[row][col].type == mat->jewels[row+1][col].type) &&
                (mat->jewels[row][col].type == mat->jewels[row+2][col].type))
            return 1;
    if (row - 2 >= 0)
        if ((mat->jewels[row][col].type == mat->jewels[row-1][col].type) &&
                (mat->jewels[row][col].type == mat->jewels[row-2][col].type))
            return 1;

    //tests if jewel is in a row sequence
    if ((col + 1 < COL_QT) && (col - 1 >= 0))
        if ((mat->jewels[row][col].type == mat->jewels[row][col+1].type) &&
                (mat->jewels[row][col].type == mat->jewels[row][col-1].type))
            return 1;
    if (col +2 < COL_QT)
        if ((mat->jewels[row][col].type == mat->jewels[row][col+1].type) &&
                (mat->jewels[row][col].type == mat->jewels[row][col+2].type))
            return 1;
    if (col - 2 >= 0)
        if ((mat->jewels[row][col].type == mat->jewels[row][col-1].type) &&
                (mat->jewels[row][col].type == mat->jewels[row][col-2].type))
            return 1;

    return 0;
}

//tests if there is any possible move left
//returns 0 if there is at least one move left
//returns 1 if game ended
int test_end_game(game_struct* mat){
    int last_row = (ROW_QT-1); 
    int last_col = (COL_QT-1); 

    for (int row = 0; row <= last_row; row++)
        for(int col = 0; col <= last_col; col++){
            if (col < last_col){
                swap_jewels_types(&mat->jewels[row][col], &mat->jewels[row][col+1]);
                if (test_jewel_mid_sequence(mat, row, col) || test_jewel_mid_sequence(mat, row, col+1)){
                    swap_jewels_types(&mat->jewels[row][col], &mat->jewels[row][col+1]);
                    return 0;
                }
                swap_jewels_types(&mat->jewels[row][col], &mat->jewels[row][col+1]);
            }

            if (row < last_row){
                swap_jewels_types(&mat->jewels[row][col], &mat->jewels[row+1][col]);
                if (test_jewel_mid_sequence(mat, row, col) || test_jewel_mid_sequence(mat, row+1, col)){
                    swap_jewels_types(&mat->jewels[row][col], &mat->jewels[row+1][col]);
                    return 0;
                }
                swap_jewels_types(&mat->jewels[row][col], &mat->jewels[row+1][col]);
            }
        }

    //if a diamond powerup exists, game does not end
    for (int row = 0; row <= last_row; row++)
        for(int col = 0; col <= last_col; col++)
            if (mat->jewels[row][col].type == WHITE) return 0;

    return 1;
}

//register user input, setting number of selected jewels and 
//the jewels chosen to be swapped
int register_user_selection(ALLEGRO_EVENT* event, game_struct* mat)
{
    vec2 clicked_slot;

    int clicked_on_jewels = (event->mouse.x >= mat->pos.x &&
            event->mouse.y >= mat->pos.y &&
            event->mouse.x <= mat->pos.x + (COL_QT*JEWEL_SIZE) &&
            event->mouse.y <= mat->pos.y + (ROW_QT*JEWEL_SIZE));

    if ( !clicked_on_jewels ) {
        mat->selected = 0; 
        return 0;
    }

    //get row and column of jewel clicked
    clicked_slot = get_rowcol(event->mouse.x, event->mouse.y, mat);

    if ( !mat->selected ) { 
        mat->selected = 1;
        mat->swap1 = &mat->jewels[clicked_slot.row][clicked_slot.col];
        return 1;
    }

    mat->selected = 2;
    mat->swap2 = &mat->jewels[clicked_slot.row][clicked_slot.col];
    return 1;
}

//set the "swap_direction" attribute as the direction from the first selected 
//jewel to the second
void set_swap_direction(game_struct *mat) {
    vec2 swap1_slot = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
    vec2 swap2_slot = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

    mat->swap_direction = STALL;
    if (swap1_slot.row == swap2_slot.row) { 
        if (swap2_slot.col == swap1_slot.col+1)
            mat->swap_direction = RIGHT;
        else if (swap2_slot.col == swap1_slot.col-1)
            mat->swap_direction = LEFT;                                                              
    }
    else if (swap1_slot.col == swap2_slot.col) { 
        if (swap2_slot.row == swap1_slot.row+1)
            mat->swap_direction = UP;                                                              
        else if (swap2_slot.row == swap1_slot.row-1)
            mat->swap_direction = DOWN;               
    }
}

//tests if there is any match 3 in the given row
int test_row(game_struct *mat, int row){
    int j, qt;
    j_type aux;

    j = 0;
    aux = mat->jewels[row][0].type;
    qt = 1;
    for (j = 1; j < COL_QT; j++)
        if ( aux == mat->jewels[row][j].type ){
            qt++;
            if (qt >= 3)
                return 1;
        }
        else{
            aux = mat->jewels[row][j].type;
            qt = 1;
        }

    return 0;
}

//tests if there is any match 3 in column col
int test_col(game_struct *mat, int col){
    int i, qt;
    j_type aux;

    i = 0;
    aux = mat->jewels[0][col].type;
    qt = 1;
    for (i = 1; i < ROW_QT; i++)
        if ( aux == mat->jewels[i][col].type ){
            qt++;
            if (qt >= 3)
                return 1;
        }
        else{
            aux = mat->jewels[i][col].type;
            qt = 1;
        }

    return 0;
}

//tests if swap should be allowed (if swap forms a matching sequence)
//returns 1 if sequence is formed by swapping
//returns 0 if no sequence is formed
int test_swap(game_struct *mat){
    vec2 rowcol;
    //dont need to test all column/row, only the sequence connected to the swap!
    rowcol = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
    if ( test_row(mat, rowcol.row) || test_col(mat, rowcol.col) )
        return 1;
    rowcol = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);
    if ( test_row(mat, rowcol.row) || test_col(mat, rowcol.col) )
        return 1;
    return 0;
}

//swap random jewels 'swap_num' times
void sort_jewels(game_struct* mat,int swap_num)
{
    int position;
    vec2 rowcol1;
    vec2 rowcol2;
    for (int i=0; i<swap_num; i++)
    {
        position = rand()%(ROW_QT*COL_QT);
        rowcol1.row = (int)(position/ROW_QT);
        rowcol1.col = (int)(position%COL_QT);

        position = rand()%(ROW_QT*COL_QT);
        rowcol2.row = (int)(position/ROW_QT);
        rowcol2.col = (int)(position%COL_QT);

        swap_jewels_types(&(mat->jewels[rowcol1.row][rowcol1.col]), &(mat->jewels[rowcol2.row][rowcol2.col]));
    }

}

int min(int a, int b)
{
    if (a<b)
        return a;
    return b;
}

jewel** allocate_jewel_matrix(int row, int col)
{
    jewel** jewels;
    if ( !(jewels = malloc(sizeof(jewel*)*row + sizeof(jewel)*row*col)) )
        return NULL;

    jewels[0] = (jewel*) (jewels + row);
    for (int i = 1; i < row; i++)
        jewels[i] = jewels[0] + (i * col);

    return jewels;
}

//initializes main game struct
int initialize_jewel_structure(game_struct *mat){

    if ( !(mat->jewels = allocate_jewel_matrix(ROW_QT, COL_QT)) )
        return 0;

    if ( !(mat->destroyed = allocate_jewel_matrix(ROW_QT, COL_QT)) )
        return 0;

    mat->pos.x = (SC_W - COL_QT*JEWEL_SIZE)/2;
    mat->pos.y = (SC_H - ROW_QT*JEWEL_SIZE)/2;

    //initialize jewels
    for (int i = 0; i < ROW_QT; i++)
        for (int j = 0; j < COL_QT; j++){
            mat->jewels[i][j].proper.x = mat->pos.x + (j * JEWEL_SIZE);
            mat->jewels[i][j].proper.y = mat->pos.y + (i * JEWEL_SIZE);
            mat->jewels[i][j].current.x = mat->jewels[i][j].proper.x;
            mat->jewels[i][j].current.y = mat->jewels[i][j].proper.y; 
            mat->jewels[i][j].power = NONE;
            mat->jewels[i][j].type = get_new_type(FIRST_AVAILABLE_JEWELS);
            mat->jewels[i][j].new_power = NONE;
            mat->jewels[i][j].new_type = EMPTY;
            mat->jewels[i][j].status = NONE;
            mat->jewels[i][j].lower = 0;
            mat->jewels[i][j].alpha = 100;

            mat->destroyed[i][j] = mat->jewels[i][j];
            mat->destroyed[i][j].alpha = 0;
        }

    mat->available_jewels = FIRST_AVAILABLE_JEWELS;
    mat->next_level_score = FIRST_SCORE_GOAL;
    mat->best_score = 0;
    mat->score = 0;
    mat->state = JEWEL;
    mat->level = 1;
    mat->selected = 0;
    mat->mistake_counter = 0;  //mistakes are counted for easter egg

    mat->swap1 = NULL;
    mat->swap2 = NULL;

    return 1;
}

//change state to new_state and records last_state game attribute
void change_state(game_struct *mat, int new_state) {
    //enters help page
    mat->last_state = mat->state;
    mat->state = new_state;
}

//start in-game timer with an interval of "sec" seconds
void set_timer(game_struct *mat, float sec){
    al_set_timer_speed(mat->timer, sec);
    al_start_timer(mat->timer);
}

//set game frame rate to an interval of "sec" seconds
void set_frame_rate(game_struct *mat, float sec){
    al_set_timer_speed(mat->fr_timer, sec);
    al_start_timer(mat->fr_timer);
}

//set frame rate to 60 FPS 
void reset_frame_rate(game_struct *mat){
    set_frame_rate(mat, 1.0/FPS);
}

//stops in-game timer (stops generating timer events)
void stop_timer(game_struct *mat) {
    al_stop_timer(mat->timer);
}

//restore music position and sets it to play
void continue_music(game_struct *mat) {
    al_play_sample_instance(mat->audio.sample_inst[BG_AUDIO]);
    al_set_sample_instance_position(mat->audio.sample_inst[BG_AUDIO], mat->audio.inst_pos[BG_AUDIO]);
    if (mat->last_state == NEW_LEVEL) {
        al_play_sample_instance(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
        al_set_sample_instance_position(mat->audio.sample_inst[NEW_LEVEL_AUDIO], mat->audio.inst_pos[NEW_LEVEL_AUDIO]);
    }
}

//saves music position and stops it
void pause_music(game_struct *mat) {
    mat->audio.inst_pos[BG_AUDIO] = al_get_sample_instance_position(mat->audio.sample_inst[BG_AUDIO]);
    al_stop_sample_instance(mat->audio.sample_inst[BG_AUDIO]);
    if (mat->state == NEW_LEVEL) {
        mat->audio.inst_pos[NEW_LEVEL_AUDIO] = al_get_sample_instance_position(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
        al_stop_sample_instance(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
    }
}
