#include "libgame.h"

vec2 get_rowcol(int x, int y, game_struct* mat)
{
    vec2 slot;
    slot.row = (y - mat->pos.y)/JEWEL_SIZE; 
    slot.col = (x - mat->pos.x)/JEWEL_SIZE;
    return slot;
}

void increase_score(game_struct* mat, int seq_size)
{
     if (seq_size == 3)
         mat->score += 10;
     else if (seq_size == 4)
         mat->score += 20;
     else if (seq_size >= 5)
         mat->score += 30;
}

void increase_score_power(game_struct* mat, int powerup)
{
     if (powerup == SQUARE)
         mat->score += 50;
     else if (powerup == STAR)
         mat->score += 60;
     else if (powerup == DIAMOND)
         mat->score += 100;
 }

j_type get_new_type(int avail_jewels){
    return (rand()%avail_jewels);
}

int test_possible_col_match(game_struct* mat, int row, int col){
    int last_col = COL_QT - 1;

    if (row > 0) {
        j_type UpRow_Col = mat->jewels[row-1][col].type;
        j_type Row_Col = mat->jewels[row][col].type;
        j_type DownRow_Col = mat->jewels[row+1][col].type;

        if (col < last_col){
            j_type UpRow_NextCol = mat->jewels[row-1][col+1].type;
            j_type Row_NextCol = mat->jewels[row][col+1].type;
            j_type DownRow_NextCol = mat->jewels[row+1][col+1].type;

            //*        *  UpRow
            // *      *   Row
            // *  or  *   DownRow
            if (((UpRow_Col == Row_NextCol) && (Row_NextCol == DownRow_NextCol)) ||
                ((UpRow_NextCol == Row_Col) && (Row_Col == DownRow_Col)))
                return 1;
            //*        *
            //*        *
            // *  or  *
            if (((UpRow_Col == Row_Col) && (Row_Col == DownRow_NextCol)) ||
                ((UpRow_NextCol == Row_NextCol) && (Row_NextCol == DownRow_Col)))
                return 1;
            //*        *
            // *      *
            //*   or   *
            if (((UpRow_Col == Row_NextCol) && (Row_NextCol == DownRow_Col)) ||
                ((UpRow_NextCol == Row_Col) && (Row_Col == DownRow_NextCol)))
                return 1;
        }
        //*        *
        //*         
        //         *
        //*   or   *
        if (row-2 >= 0)
            if (((UpRow_Col == DownRow_Col) && (DownRow_Col == mat->jewels[row-2][col].type)) ||
                ((Row_Col == DownRow_Col) && (DownRow_Col == mat->jewels[row-2][col].type)))
                return 1;
    }
    return 0;
}

int test_possible_row_match(game_struct* mat, int row, int col){
    int last_col = COL_QT - 1;

    if (col < last_col){
        j_type Row_PrevCol = mat->jewels[row][col-1].type;
        j_type Row_Col = mat->jewels[row][col].type;
        j_type Row_NextCol = mat->jewels[row][col+1].type;

        if (Row_PrevCol == WHITE)
            return 1;

        if (row){
            j_type UpRow_PrevCol = mat->jewels[row-1][col-1].type;
            j_type UpRow_Col = mat->jewels[row-1][col].type;
            j_type UpRow_NextCol = mat->jewels[row-1][col+1].type;

            //*          *  UpRow 
            // **  or  **   Row   
            if (((UpRow_PrevCol == Row_Col) && (Row_Col == Row_NextCol)) ||
                ((Row_PrevCol == Row_Col) && (Row_Col == UpRow_NextCol)))
                return 1;
            // **      **
            //*    or    *
            if (((Row_PrevCol == UpRow_Col) && (UpRow_Col == UpRow_NextCol)) ||
                ((UpRow_PrevCol == UpRow_Col) && (UpRow_Col == Row_NextCol)))
                return 1;
            //* *       *  
            // *   or  * *
            if (((UpRow_PrevCol == Row_Col) && (Row_Col == UpRow_NextCol)) || 
                ((Row_PrevCol == UpRow_Col) && (UpRow_Col == Row_NextCol)))
                return 1;
        }

        //* **  or  ** * 
        if ((col+2) <= last_col){
            if ((Row_NextCol == WHITE) || (mat->jewels[row][col+2].type == WHITE))
                return 1;
             
            if (((Row_PrevCol == Row_NextCol) && (Row_NextCol == mat->jewels[row][col+2].type)) ||
                ((Row_PrevCol == Row_Col) && (Row_Col == mat->jewels[row][col+2].type)))
                return 1;
        }
    }

    return 0;
}

int test_end_game(game_struct* mat){
    int last_row = (ROW_QT-1); 
    int last_col = (COL_QT-1); 

    for (int row = last_row; row >= 0; row--) 
        for(int col = 0; col <= last_col; col++){
            if (test_possible_row_match(mat, row, col+1) ||
                test_possible_col_match(mat, row-1, col))
                return 0;
        }

    return 1;
}

int register_user_input(ALLEGRO_EVENT* event, game_struct* mat)
{
    //if clicked on jewel matrix
    if (event->mouse.x >= mat->pos.x &&
        event->mouse.y >= mat->pos.y &&
        event->mouse.x <= mat->pos.x + (COL_QT*JEWEL_SIZE) &&
        event->mouse.y <= mat->pos.y + (ROW_QT*JEWEL_SIZE))
    {
        //get row and column of jewel clicked
        vec2 clicked_slot;
        clicked_slot = get_rowcol(event->mouse.x, event->mouse.y, mat);

        if (!mat->selected)
        {
            //first jewel to be swapped is now selected
            mat->selected = 1;
            mat->swap1 = &mat->jewels[clicked_slot.row][clicked_slot.col];
        }
        else
        {
            mat->selected = 0;
            //if clicked at the side of first selected jewel
            if (event->mouse.y > (mat->swap1->proper.y+1) &&
                event->mouse.y < (mat->swap1->proper.y+JEWEL_SIZE-1) )
            {
                //swap right 
                if (event->mouse.x > (mat->swap1->proper.x+JEWEL_SIZE) &&
                    event->mouse.x < (mat->swap1->proper.x+(2*JEWEL_SIZE)))
                {
                    mat->swap2 = &mat->jewels[clicked_slot.row][clicked_slot.col];
                    swap_jewels(mat->swap1, mat->swap2, -VELOCITY, 0);
                    return 1;
                }
                //swap left
                else if (event->mouse.x > mat->swap1->proper.x-JEWEL_SIZE &&
                        event->mouse.x < mat->swap1->proper.x)
                {
                    mat->swap2 = &mat->jewels[clicked_slot.row][clicked_slot.col];
                    swap_jewels(mat->swap1, mat->swap2, VELOCITY, 0);
                    return 1;
                }
            }
            //if clicked at top or bottom of first selected jewel
            else if (event->mouse.x > (mat->swap1->proper.x+1) &&
                    event->mouse.x < (mat->swap1->proper.x+JEWEL_SIZE-1))
            {
                //swap up
                if (event->mouse.y > mat->swap1->proper.y-JEWEL_SIZE &&
                    event->mouse.y < mat->swap1->proper.y)
                {
                    mat->swap2 = &mat->jewels[clicked_slot.row][clicked_slot.col];
                    swap_jewels(mat->swap1, mat->swap2, 0, VELOCITY);
                    return 1;
                }
                //swap down
                else if (event->mouse.y > mat->swap1->proper.y+JEWEL_SIZE &&
                        event->mouse.y < mat->swap1->proper.y+2*JEWEL_SIZE )
                {
                    mat->swap2 = &mat->jewels[clicked_slot.row][clicked_slot.col];
                    swap_jewels(mat->swap1, mat->swap2, 0, -VELOCITY);
                    return 1;
                }
            }
        }
    }
    else
        mat->selected = 0; //if click outside matrix, cancel swapping

    return 0;
}

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

jewel** allocate_jewel_matrix(int row, int col)
{
    jewel** jewels;
    if ( !(jewels = malloc(sizeof(jewel*)*ROW_QT + sizeof(jewel)*ROW_QT*COL_QT)) )
        return NULL;

    jewels[0] = (jewel*) (jewels + ROW_QT);
    for (int i = 1; i < ROW_QT; i++)
        jewels[i] = jewels[0] + (i * COL_QT);

    return jewels;
}

int initialize_jewel_structure(game_struct *mat){

    if ( !(mat->jewels = allocate_jewel_matrix(ROW_QT, COL_QT)) )
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
        }

    mat->available_jewels = FIRST_AVAILABLE_JEWELS;
    mat->next_level_score = FIRST_SCORE_GOAL;
    mat->best_score = 0;
    mat->score = 0;
    mat->state = JEWEL;
    mat->level = 1;
    mat->selected = 0;

    mat->swap1 = NULL;
    mat->swap2 = NULL;

    return 1;
}


int min(int a, int b)
{
    if (a<b)
        return a;
    return b;
}
