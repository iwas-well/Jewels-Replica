#include "structs.h"
#include "jewels_destruction.h"
#include "allegro_dependencies.h"
#include "libgame.h"

/*destroys jewels in given row and column
  (destroyer_type used in case a diamond is hit)*/
void star_destroyer(game_struct* mat, int row, int col, j_type destroyer_type) {
    //destroy row
    for (int j = 0; j < COL_QT; j++)
        destroy_jewel_on_slot(mat,row,j, destroyer_type);

    //destroy column
    for (int i = 0; i < ROW_QT; i++)
        destroy_jewel_on_slot(mat, i, col, destroyer_type);
}

/*destroys jewels around given row and column
  (destroyer_type used in case a diamond is hit)*/
void square_destroyer(game_struct* mat, int row, int col, j_type destroyer_type) {
    int aux_row;
    for (int i = 0; i < 3; i++) {
        aux_row = (row-1)+i;

        if ((aux_row < ROW_QT) && ( aux_row >= 0)){

            //destroy jewels on the line above
            if ((col-1) >= 0)
                destroy_jewel_on_slot(mat, (row-1)+i, col-1, destroyer_type);

            //destroy jewels on the line in the middle
            destroy_jewel_on_slot(mat, (row-1)+i, col, destroyer_type);

            //destroy jewels on the line below
            if (col+1 < COL_QT)
                destroy_jewel_on_slot(mat, (row-1)+i, col+1, destroyer_type);
        }
    }
}

//destroys jewels of type "destroy type"
void diamond_destroyer(game_struct *mat, j_type destroy_type) {
    for (int i = 0; i < ROW_QT; i++)
        for (int j = 0; j < COL_QT; j++)
            if (mat->jewels[i][j].type == destroy_type)
                destroy_jewel_on_slot(mat, i, j, destroy_type);
}

/*returns size of col sequence from up to bottom starting at
  jewel with given row,col*/
int get_col_sequence_size(game_struct* mat, int row, int col) {
    j_type aux = mat->jewels[row][col].type;
    int seq = 1;

    for (int i = row+1; i < ROW_QT; i++)
        if ( aux == mat->jewels[i][col].type )
            seq++;
        else
            return seq;

    return seq;
}

/*returns size of row sequence from left to right starting at
  jewel with given row, col*/
int get_row_sequence_size(game_struct* mat, int row, int col) {
    j_type aux = mat->jewels[row][col].type;
    int seq = 1;

    for (int j = col+1; j < COL_QT; j++)
        if ( aux == mat->jewels[row][j].type )
            seq++;
        else
            return seq;

    return seq;
}

/*Destroys jewel in given "row", "col" and increases the player score.
  Copies the jewel to the "mat->destroyed" jewel matrix,
  sets jewel type as empty and removes its status and power.
  If jewel had a power, destroys other jewels accordingly*/
void destroy_jewel_on_slot(game_struct* mat, int row, int col, j_type destroy_type) {
    if (mat->jewels[row][col].type == EMPTY)
        return;

    int aux_power;
    int aux_type;
    aux_power = mat->jewels[row][col].power;
    aux_type = mat->jewels[row][col].type;

    mat->destroyed[row][col] = mat->jewels[row][col]; //copy jewel to destroyed matrix
    mat->destroyed[row][col].alpha = 100; //sets its initial transparency

    //destroy jewel
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
void destroy_jewels(game_struct* mat) {
    for (int row = 0; row < ROW_QT; row++)
        for (int col = 0; col < COL_QT; col++)
            if ( mat->jewels[row][col].status == DESTROY )
                destroy_jewel_on_slot(mat, row, col, EMPTY);
}

//sets new "type" and "power" attribute of jewel in given "row", "col"
void create_new_power_on_slot(game_struct* mat, int row, int col) {
    switch (mat->jewels[row][col].new_power) { 
        case STAR:
        case SQUARE:
        case DIAMOND:
            mat->jewels[row][col].type = mat->jewels[row][col].new_type;
            mat->jewels[row][col].power = mat->jewels[row][col].new_power;
            break;
        default:
            break;
    }

    mat->jewels[row][col].new_type = EMPTY;
    mat->jewels[row][col].new_power = NONE;
}

//sets new "type" and "power" attribute of all jewels
void create_new_powers(game_struct* mat) {
    for (int row = 0; row <ROW_QT; row++)
        for (int col = 0; col <COL_QT; col++)
            if ( mat->jewels[row][col].new_power != NONE )
                create_new_power_on_slot(mat, row, col);
}

static int create_row_sequence_powerups(game_struct *mat) {
    int row, col, seq;
    int has_jewel_sequence = 0;

    for (row = 0; row < ROW_QT; row++) {
        col = 0;
        while (col < COL_QT) {
            seq = get_row_sequence_size(mat,row,col);

            if (seq < 3) {
                col = col+seq;
                continue;
            }

            has_jewel_sequence = 1;

            for (int k = 0; k < seq; k++)
                mat->jewels[row][col+k].status = DESTROY;

            if ( !(mat->swap2) ) {
                col = col+seq;
                continue;
            }

            vec2 slot_swap1 = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
            vec2 slot_swap2 = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

            if (seq == 3)
                increase_score(mat, seq);
            else if (seq == 4) {
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
                if ((slot_swap1.row == row) && (slot_swap1.col >= col) && (slot_swap1.col <= col+seq-1)) {
                    mat->jewels[row][slot_swap1.col].new_type = WHITE;
                    mat->jewels[row][slot_swap1.col].new_power = DIAMOND;
                }
                else if ((slot_swap2.row == row) && (slot_swap2.col >= col) && (slot_swap2.col <= col+seq-1)) {
                    mat->jewels[row][slot_swap2.col].new_type = WHITE;
                    mat->jewels[row][slot_swap2.col].new_power = DIAMOND;
                }
                else {
                    mat->jewels[row][col+seq/2].new_type = WHITE;
                    mat->jewels[row][col+seq/2].new_power = DIAMOND;
                }
                increase_score(mat, seq);
            }
            col = col+seq;
        }
    }

    return has_jewel_sequence;
}

int get_free_destroyed_on_column(game_struct *mat, int row, int col) {
    for (int i=col; i>=0; i--) {
        if (mat->jewels[row][i].type == mat->jewels[row][col].type) {
            if ((mat->jewels[row][i].status == DESTROY) && 
                    (mat->jewels[row][i].new_power == NONE)) 
                return i;
        }
        else
            break;
    }

    for (int i=col+1; i<COL_QT; i++) {
        if (mat->jewels[row][i].type == mat->jewels[row][col].type) {
            if ((mat->jewels[row][i].status == DESTROY) && 
                    (mat->jewels[row][i].new_power == NONE)) 
                return i;
        }
        else
            break;
    }

    return -1;
}


static int create_col_sequence_powerups(game_struct *mat) {
    int row, col, seq;
    int has_jewel_sequence = 0;

    for (col = 0; col < COL_QT; col++) {
        row = 0;
        while (row < ROW_QT) {
            seq = get_col_sequence_size(mat,row,col);

            if (seq < 3) {
                row = row+seq;
                continue;
            }

            has_jewel_sequence = 1;

            //checks if in the sequence exists a jewel already destroyed
            //which means the sequence is crossing another and,
            //in this case, puts a star power up on the intersection
            for (int k = 0; k < seq; k++) {
                if (mat->jewels[row+k][col].status == DESTROY) {
                        int free_col = get_free_destroyed_on_column(mat, row+k, col);
                        if (free_col != -1) {
                            mat->jewels[row+k][free_col].new_type = mat->jewels[row+k][col].new_type;
                            mat->jewels[row+k][free_col].new_power = mat->jewels[row+k][col].new_power;
                        }
                        //buscar peça vazia na sequencia?    
                        mat->jewels[row+k][col].new_power = STAR;
                    //}
                    mat->jewels[row+k][col].new_type = mat->jewels[row+k][col].type;
                }
                mat->jewels[row+k][col].status = DESTROY;
            }

            if ( !(mat->swap2) ) {
                row = row+seq;
                continue;
            }

            vec2 slot_swap1 = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
            vec2 slot_swap2 = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

            if (seq == 3)
                increase_score(mat, seq);
            else if (seq == 4) {
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
                if ((slot_swap1.col == col) && (slot_swap1.row >= row) && (slot_swap1.row <= row+seq-1)) {
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
                else if ((slot_swap2.col == col) && (slot_swap2.row >= row) && (slot_swap2.row <= row+seq-1)) {
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
                    if (mat->jewels[row+seq/2][col].new_power == NONE) {
                        //coloca powerup no meio da sequencia
                        mat->jewels[row+seq/2][col].new_type = WHITE;
                        mat->jewels[row+seq/2][col].new_power = DIAMOND;
                    }
                    else {
                        //coloca powerup no inicio da sequencia
                        mat->jewels[row+seq-1][col].new_type = WHITE;
                        mat->jewels[row+seq-1][col].new_power = DIAMOND;
                    }
                }
                increase_score(mat, seq);
            }//if seq >=5 
            row = row+seq;
        }
    }

    return has_jewel_sequence;
}

/*sets "status" attribute of jewels to be destroyed as "DESTROY",
  count same type sequences sizes and increases score accordingly
  and creates powerups*/
int set_to_destroy_matched_jewels(game_struct* mat) {
    int has_jewel_sequence;

    has_jewel_sequence = create_row_sequence_powerups(mat);
    has_jewel_sequence += create_col_sequence_powerups(mat);

    if (has_jewel_sequence) {
        destroy_jewels(mat);
        create_new_powers(mat);
    }

    return has_jewel_sequence;
}
