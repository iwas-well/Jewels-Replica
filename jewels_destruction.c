#include "jewels_destruction.h"

void star_destroyer(game_struct* mat, int row, int col, int destroy_type)
{
    //destroy row
    for (int j = 0; j < COL_QT; j++)
        destroy_jewel_on_slot(mat,row,j, destroy_type);

    //destroy column
    for (int i = 0; i < ROW_QT; i++)
        destroy_jewel_on_slot(mat, i, col, destroy_type);
}

void diamond_destroyer(game_struct *mat, j_type type)
{
    mat->score += 100;
    for (int i = 0; i < ROW_QT; i++)
        for (int j = 0; j < COL_QT; j++)
            if (mat->jewels[i][j].type == type)
                destroy_jewel_on_slot(mat, i, j, WHITE);
}

void square_destroyer(game_struct* mat, int row, int col, int destroy_type)
{
    int aux_row;
    for (int i = 0; i < 3; i++) {
        aux_row = (row-1)+i;

        if ((aux_row < ROW_QT) && ( aux_row >= 0)){

            //destroy jewels on the line above
            if ((col-1) >= 0)
                destroy_jewel_on_slot(mat, (row-1)+i, col-1, destroy_type);

            //destroy jewels on the line in the middle
            destroy_jewel_on_slot(mat, (row-1)+i, col, destroy_type);

            //destroy jewels on the line below
            if (col+1 < COL_QT)
                destroy_jewel_on_slot(mat, (row-1)+i, col+1, destroy_type);
        }
    }
}

//returns size of col sequence from up to bottom starting at
//jewel with given row,col
int col_sequence_size(game_struct* mat, int row, int col)
{
    j_type aux = mat->jewels[row][col].type;
    int seq = 1;

    for (int i = row+1; i < ROW_QT; i++)
        if ( aux == mat->jewels[i][col].type )
            seq++;
        else
            return seq;

    return seq;
}

//returns size of row sequence from left to right starting at
//jewel with given row,col
int row_sequence_size(game_struct* mat, int row, int col)
{
    j_type aux = mat->jewels[row][col].type;
    int seq = 1;

    for (int j = col+1; j < COL_QT; j++)
        if ( aux == mat->jewels[row][j].type )
            seq++;
        else
            return seq;

    return seq;
}

//destroy jewel of given row, col
void destroy_jewel_on_slot(game_struct* mat, int row, int col, int destroy_type)
{
    if (mat->jewels[row][col].type == EMPTY)
        return;

    int aux_power;
    int aux_type;
    aux_power = mat->jewels[row][col].power;
    aux_type = mat->jewels[row][col].type;

    //-----------------
    mat->vanish[row][col] = mat->jewels[row][col]; //copy jewel to vanishing matrix
    mat->vanish[row][col].alpha = 100;
    //-----------------

    mat->jewels[row][col].type = EMPTY;
    mat->jewels[row][col].status = NONE;
    mat->jewels[row][col].power = NONE;

    if (aux_power == STAR){
        increase_score_power(mat, SQUARE);
        star_destroyer(mat, row, col, aux_type);
    }
    else if (aux_power == SQUARE){
        increase_score_power(mat, SQUARE);
        square_destroyer(mat, row, col, aux_type);
    }
    else if (aux_power == DIAMOND){
        increase_score_power(mat, DIAMOND);
        diamond_destroyer(mat, destroy_type);
    }
}

//destroy all jewels with status set as destroy
void destroy_jewels(game_struct* mat)
{
    for (int row = 0; row < ROW_QT; row++)
        for (int col = 0; col < COL_QT; col++)
            if ( mat->jewels[row][col].status == DESTROY )
                destroy_jewel_on_slot(mat, row, col, EMPTY);
}

void create_new_power_on_slot(game_struct* mat, int row, int col) {
    if( mat->jewels[row][col].new_power == STAR) {
        mat->jewels[row][col].type = mat->jewels[row][col].new_type;
        mat->jewels[row][col].power = STAR;
    }
    else if( mat->jewels[row][col].new_power == SQUARE) {
        mat->jewels[row][col].type = mat->jewels[row][col].new_type;
        mat->jewels[row][col].power = SQUARE;
    }
    else if( mat->jewels[row][col].new_power == DIAMOND) {
        mat->jewels[row][col].type = mat->jewels[row][col].new_type;
        mat->jewels[row][col].power = DIAMOND;
    }

    mat->jewels[row][col].new_type = EMPTY;
    mat->jewels[row][col].new_power = NONE;
}

void create_new_powers(game_struct* mat) {
    for (int row = 0; row <ROW_QT; row++)
        for (int col = 0; col <COL_QT; col++)
            if ( mat->jewels[row][col].new_power != NONE )
                create_new_power_on_slot(mat, row, col);
}

//set jewels to be destroyed and creates powerups
int set_to_destroy_matched_jewels(game_struct* mat) {
    int jewel_match = 0;
    int row, col, seq;
    row = 0;
    while (row < ROW_QT) {
        col = 0;
        while (col < COL_QT) {
            seq = row_sequence_size(mat,row,col);
            if (seq >= 3) {
                jewel_match = 1;
                for (int k = 0; k < seq; k++)
                    mat->jewels[row][col+k].status = DESTROY;
                
                if (mat->swap2) { //impede que powerups sejam criados e score seja contabilizado antes do input
                    vec2 slot_swap1 = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    vec2 slot_swap2 = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

                    if (seq == 3)
                        increase_score(mat, seq);

                    if (seq == 4) {
                        //se swap1 pertence a sequencia, ele vira powerup
                        if ((slot_swap1.row == row) && (slot_swap1.col <= col+seq-1) && (slot_swap1.col >= col)) {
                            mat->jewels[row][slot_swap1.col].new_type = mat->jewels[row][slot_swap1.col].type;
                            mat->jewels[row][slot_swap1.col].new_power = SQUARE;
                        }
                        //se swap2 pertence a sequencia, ele vira powerup
                        else if ((slot_swap2.row == row) && (slot_swap2.col <= col+seq-1) && (slot_swap2.col >= col)) {
                            mat->jewels[row][slot_swap2.col].new_type = mat->jewels[row][slot_swap2.col].type;
                            mat->jewels[row][slot_swap2.col].new_power = SQUARE;
                        }
                        //se nem swap1 nem swap2 pertencem, peca do meio vira powerup
                        else {
                            mat->jewels[row][col+1].new_type = mat->jewels[row][col+1].type;
                            mat->jewels[row][col+1].new_power = SQUARE;
                        }
                        increase_score(mat, seq);
                    }
                    else if (seq >= 5) {
                        if ((slot_swap1.row == row) && (slot_swap1.col <= col+seq-1) && (slot_swap1.col >= col)) {
                            mat->jewels[row][slot_swap1.col].new_type = WHITE;
                            mat->jewels[row][slot_swap1.col].new_power = DIAMOND;
                        }
                        else if ((slot_swap2.row == row) && (slot_swap2.col <= col+seq-1) && (slot_swap2.col >= col)) {
                            mat->jewels[row][slot_swap2.col].new_type = WHITE;
                            mat->jewels[row][slot_swap2.col].new_power = DIAMOND;
                        }
                        else {
                            mat->jewels[row][col+2].new_type = WHITE;
                            mat->jewels[row][col+2].new_power = DIAMOND;
                        }
                        increase_score(mat, seq);
                    }
                }
            }
            col = col+seq;
        }
        row++;
    }

    col = 0;
    while ( col < COL_QT ) {
        row = 0;
        while (row < ROW_QT) {
            seq = col_sequence_size(mat,row,col);
            if (seq >= 3) {
                jewel_match = 1;
                for (int k = 0; k < seq; k++) {
                    if (mat->jewels[row+k][col].status == DESTROY) {
                        if ( mat->jewels[row+k][col].new_power == NONE ) {
                            mat->jewels[row+k][col].new_power = STAR;
                        }
                        else {
                            //se ja ha powerup na intercecao, troca por star e o coloca na ponta oposta
                            //talvez trocar futuramente pelo inicio da coluna
                            mat->jewels[row+(seq-1)-k][col].new_type = mat->jewels[row+k][col].type;
                            mat->jewels[row+(seq-1)-k][col].new_power = mat->jewels[row+k][col].new_power;
                            //mat->jewels[row+(seq-1)-k][col].new_power = aux_power;

                            //buscar peça vazia na sequencia?    
                            mat->jewels[row+k][col].new_power = STAR;
                        }
                        mat->jewels[row+k][col].new_type = mat->jewels[row+k][col].type;
                    }
                    mat->jewels[row+k][col].status = DESTROY;
                }

                if (mat->swap2) {
                    vec2 slot_swap1 = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    vec2 slot_swap2 = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

                    if (seq == 3) {
                        increase_score(mat, seq);
                    }
                    if (seq == 4) {
                        if ((slot_swap1.col == col) && (slot_swap1.row <= row+seq-1) && (slot_swap1.row >= row)) {
                            if (mat->jewels[slot_swap1.row][col].new_power == NONE){
                                mat->jewels[slot_swap1.row][col].new_type = mat->jewels[slot_swap1.row][col].type;
                                mat->jewels[slot_swap1.row][col].new_power = SQUARE;
                            }
                            else {
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row+seq-1][col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                        else if ((slot_swap2.col == col) && (slot_swap2.row <= row+seq-1) && (slot_swap2.row >= row)) {
                            if (mat->jewels[slot_swap2.row][col].new_power == NONE) {
                                mat->jewels[slot_swap2.row][col].new_type = mat->jewels[slot_swap2.row][col].type;
                                mat->jewels[slot_swap2.row][col].new_power = SQUARE;
                            }
                            else {
                                //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row+seq-1][col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                        else {
                            if (mat->jewels[row+1][col].new_power == NONE) {
                            //coloca powerup no meio da sequencia
                                mat->jewels[row+1][col].new_type = mat->jewels[row+1][col].type;
                                mat->jewels[row+1][col].new_power = SQUARE;
                            }
                            else {
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row+seq-1][col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                        increase_score(mat, seq);
                    }
                    else if (seq >= 5) {
                        if ((slot_swap1.col == col) && (slot_swap1.row <= row+seq-1) && (slot_swap1.row >= row)) {
                            if (mat->jewels[slot_swap1.row][col].new_power == NONE) {
                                mat->jewels[slot_swap1.row][col].new_type = WHITE;
                                mat->jewels[slot_swap1.row][col].new_power = DIAMOND;
                            }
                            else {
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = WHITE;
                                mat->jewels[row+seq-1][col].new_power = DIAMOND;
                            }
                        }
                        else if ((slot_swap2.col == col) && (slot_swap2.row <= row+seq-1) && (slot_swap2.row >= row)) {
                            if (mat->jewels[slot_swap2.row][col].new_power == NONE) {
                                mat->jewels[slot_swap2.row][col].new_type = WHITE;
                                mat->jewels[slot_swap2.row][col].new_power = DIAMOND;
                            }
                            else {
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = WHITE;
                                mat->jewels[row+seq-1][col].new_power = DIAMOND;
                            }
                        }
                        else {
                            if (mat->jewels[row+2][col].new_power == NONE) {
                            //coloca powerup no meio da sequencia
                                mat->jewels[row+2][col].new_type = WHITE;
                                mat->jewels[row+2][col].new_power = DIAMOND;
                            }
                            else {
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = WHITE;
                                mat->jewels[row+seq-1][col].new_power = DIAMOND;
                            }
                        }
                        increase_score(mat, seq);
                    }//if seq >=5 
                }//if mat->swap2
            } //if seq >= 3
            row = row+seq;
        }
        col++;
    }

    if (jewel_match) {
        destroy_jewels(mat);
        create_new_powers(mat);
    }

    return jewel_match;
}
