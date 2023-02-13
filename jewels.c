#include <allegro5/color.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/display.h>
#include <allegro5/events.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

//todo:
//  refactor code
//  rock
//  destruction animation

//game configuration
//#define SC_W        1300 //screen width
//#define SC_H        750  //screen height
#define SC_W        1000 //screen width
#define SC_H        750  //screen height
#define ROW_QT      8    //quantity of lines on the game matrix
#define COL_QT      8    //quantity of columns on the game matrix
#define JEWEL_SIZE  65   //lenght of jewel slot
#define VELOCITY    4    //jewel movement velocity
#define FIRST_SCORE_GOAL    500
#define NEW_LEVEL_TIMER     30 //num of iterations on new level state
#define SORTED_PER_FRAME    5  //num of swappings per frame on new level state
#define NEW_LEVEL_SLOW_DOWN 3  //amount of times new level state is slown by
#define FIRST_AVAILABLE_JEWELS 5 //number of jewels available in first level
#define MAX_AVAILABLE_JEWELS   6
                         
//game states
#define INPUT       0
#define JEWEL       1
#define DROP        2
#define SWAP        3
#define END_GAME    4
#define PAUSE       5
#define NEW_LEVEL   6
#define HELP_PAGE   7

//status
#define NONE            0
#define DESTROY         1
#define DESTROY_STAR    2
#define DESTROY_SQUARE  3

//powerup
#define NONE        0
#define STAR        1
#define SQUARE      2
#define DIAMOND     3

//diamond type
typedef enum{RED, BLUE, GREEN, YELLOW, PURPLE, GREY, WHITE, EMPTY = -1} j_type;

typedef union{ //allows calling 'x' variable by 'col', and 'y' by 'row'
    struct{ int x, y; };
    struct{ int col, row; };
} vec2;

typedef struct{
    float x, y;
} fvec2;

typedef struct jewel{
    vec2 current;   //current xy position of jewel image
    vec2 proper;    //xy position of jewel slot
    vec2 vel;       //jewel xy velocity values
    int new_power;
    j_type new_type;
    int power;    
    int status;
    j_type type;    //jewel color/type
    int lower;      //flag for how many positions the jewel should lower
} jewel;

typedef struct game_images{
    ALLEGRO_BITMAP** base_sprite;
    ALLEGRO_BITMAP** up_sprite;
    ALLEGRO_BITMAP** state_screen;
} game_images;

typedef struct game_audio{
    ALLEGRO_SAMPLE** sample;
    ALLEGRO_SAMPLE_INSTANCE** instance;
    unsigned int** instance_pos;
} game_audio;

typedef struct jmat{
    jewel **jewels;         //jewel matrix
    jewel *swap1, *swap2;   //last two swaped jewels
    vec2 pos;               //xy position of jewel matrix
    int score;
    int available_jewels;   //num of jewels available in current level
    game_images image;
    game_audio audio;
} jmat;

void destroy_jewel(jmat* mat, int row, int col, int destroyer_type);

vec2 get_rowcol(int x, int y, jmat* mat)
{
    vec2 slot;
    slot.row = (y - mat->pos.y)/JEWEL_SIZE; 
    slot.col = (x - mat->pos.x)/JEWEL_SIZE;
    return slot;
}

void must_init(bool test, const char *description)
{
    if(test) return;
    printf("Nao foi possivel inicializar %s\n", description);
    exit(1);
}

void star_destroyer(jmat* mat, int row, int col, int destroyer_type)
{
    //destroi linha
    for (int j = 0; j < COL_QT; j++)
        destroy_jewel(mat,row,j, destroyer_type);

    //destroi coluna
    for (int i = 0; i < ROW_QT; i++)
        destroy_jewel(mat, i, col, destroyer_type);
}

void diamond_destroyer(jmat *mat, j_type type)
{
    int i, j;

    mat->score += 100;
    for (i = 0; i < ROW_QT; i++)
        for (j = 0; j < COL_QT; j++)
            if (mat->jewels[i][j].type == type)
                destroy_jewel(mat, i, j, WHITE);
}

void square_destroyer(jmat* mat, int row, int col, int destroyer_type)
{
    int aux_row;
    for (int i = 0; i < 3; i++) {
        aux_row = (row-1)+i;

        if ((aux_row < ROW_QT) && ( aux_row >= 0)){

            //destroi jewels na linha de acima
            if ((col-1) >= 0)
                destroy_jewel(mat, (row-1)+i, col-1, destroyer_type);

            //destroi jewels da linha do meio
            destroy_jewel(mat, (row-1)+i, col, destroyer_type);

            //destroi jewels na linha de baixo
            if (col+1 < COL_QT)
                destroy_jewel(mat, (row-1)+i, col+1, destroyer_type);
        }
    }
}

int col_sequence_size(jmat* mat, int row, int col)
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

//returns size of row sequence from left to right starting at jewel
int row_sequence_size(jmat* mat, int row, int col)
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

void destroy_jewels(jmat* mat)
{
    for (int row = 0; row < ROW_QT; row++)
        for (int col = 0; col < COL_QT; col++)
            if ( mat->jewels[row][col].status == DESTROY )
                destroy_jewel(mat, row, col, EMPTY);
}
void increase_score(jmat* mat, int seq_size)
{
     if (seq_size == 3)
         mat->score += 10;
     else if (seq_size == 4)
         mat->score += 20;
     else if (seq_size >= 5)
         mat->score += 30;
}

void increase_score_power(jmat* mat, int powerup)
{
     if (powerup == SQUARE)
         mat->score += 50;
     else if (powerup == STAR)
         mat->score += 60;
     else if (powerup == DIAMOND)
         mat->score += 100;
 }

void destroy_jewel(jmat* mat, int row, int col, int destroyer_type)
{
    int aux_power;
    int aux_type;
    aux_power = mat->jewels[row][col].power;
    aux_type = mat->jewels[row][col].type;

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
        diamond_destroyer(mat, destroyer_type);
    }

}

void create_new_powers(jmat* mat){
    for (int row = 0; row <ROW_QT; row++)
        for (int col = 0; col <COL_QT; col++)
            if ( mat->jewels[row][col].new_power != NONE ){

                if( mat->jewels[row][col].new_power == STAR){
                    mat->jewels[row][col].type = mat->jewels[row][col].new_type;
                    mat->jewels[row][col].power = STAR;
                }
                else if( mat->jewels[row][col].new_power == SQUARE){
                    mat->jewels[row][col].type = mat->jewels[row][col].new_type;
                    mat->jewels[row][col].power = SQUARE;
                }
                else if( mat->jewels[row][col].new_power == DIAMOND){
                    mat->jewels[row][col].type = mat->jewels[row][col].new_type;
                    mat->jewels[row][col].power = DIAMOND;
                }

                mat->jewels[row][col].new_type = EMPTY;
                mat->jewels[row][col].new_power = NONE;
            }
}

int set_to_destroy_matched_jewels(jmat* mat)
{
    //!create flag not destroy for new power jewels!
    //jewels should have a power 'flag' and a 'new power' flag
    int match = 0;
    int row, col, seq;
    j_type aux;
    row = 0;
    while ( row < ROW_QT ){
        col = 0;
        while (col < COL_QT){
            seq = row_sequence_size(mat,row,col);
            if (seq >= 3){
                match = 1;
                for (int k = 0; k < seq; k++)
                    mat->jewels[row][col+k].status = DESTROY;

                
                if (mat->swap2){ //impede que powerups sejam criados e score seja contabilizado antes do input
                    vec2 slot_swap1 = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    vec2 slot_swap2 = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

                    if (seq == 3){
                        increase_score(mat, seq);
                    }
                    if (seq == 4)
                    {
                        //se swap1 pertence a sequencia, ele vira powerup
                        if ((slot_swap1.row == row) && (slot_swap1.col <= col+seq-1) && (slot_swap1.col >= col)) 
                        {
                            mat->jewels[row][slot_swap1.col].new_type = mat->jewels[row][slot_swap1.col].type;
                            mat->jewels[row][slot_swap1.col].new_power = SQUARE;
                        }
                        //se swap2 pertence a sequencia, ele vira powerup
                        else if ((slot_swap2.row == row) && (slot_swap2.col <= col+seq-1) && (slot_swap2.col >= col))
                        {
                            mat->jewels[row][slot_swap2.col].new_type = mat->jewels[row][slot_swap2.col].type;
                            mat->jewels[row][slot_swap2.col].new_power = SQUARE;
                        }
                        //se nem swap1 nem swap2 pertencem, peca do meio vira powerup
                        else
                        {
                            mat->jewels[row][col+1].new_type = mat->jewels[row][col+1].type;
                            mat->jewels[row][col+1].new_power = SQUARE;
                        }
                        increase_score(mat, seq);
                    }
                    else if (seq >= 5)
                    {
                        if ((slot_swap1.row == row) && (slot_swap1.col <= col+seq-1) && (slot_swap1.col >= col)) 
                        {
                            mat->jewels[row][slot_swap1.col].new_type = WHITE;
                            mat->jewels[row][slot_swap1.col].new_power = DIAMOND;
                        }
                        else if ((slot_swap2.row == row) && (slot_swap2.col <= col+seq-1) && (slot_swap2.col >= col))
                        {
                            mat->jewels[row][slot_swap2.col].new_type = WHITE;
                            mat->jewels[row][slot_swap2.col].new_power = DIAMOND;
                        }
                        else
                        {
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
    while ( col < COL_QT ){
        row = 0;
        while (row < ROW_QT){
            seq = col_sequence_size(mat,row,col);
            if (seq >= 3)
            {
                match = 1;
                for (int k = 0; k < seq; k++){
                    if (mat->jewels[row+k][col].status == DESTROY){
                        if ( mat->jewels[row+k][col].new_power == NONE ){
                            mat->jewels[row+k][col].new_power = STAR;
                        }
                        else{
                            //se ja ha powerup na intercecao, troca por star e o coloca na ponta oposta
                            //talvez trocar futuramente pelo inicio da coluna
                            mat->jewels[row+(seq-1)-k][col].new_type = mat->jewels[row+k][col].type;
                            mat->jewels[row+(seq-1)-k][col].new_power = mat->jewels[row+k][col].new_power;
                            //mat->jewels[row+(seq-1)-k][col].new_power = aux_power;

                            //buscar peca vazia na sequencia?    
                            mat->jewels[row+k][col].new_power = STAR;
                        }
                        mat->jewels[row+k][col].new_type = mat->jewels[row+k][col].type;
                    }
                    mat->jewels[row+k][col].status = DESTROY;
                }

                if (mat->swap2)
                {
                    //vec2 rowcol = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    vec2 slot_swap1 = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    vec2 slot_swap2 = get_rowcol(mat->swap2->current.x, mat->swap2->current.y, mat);

                    if (seq == 3){
                        increase_score(mat, seq);
                    }
                    if (seq == 4)
                    {
                        if ((slot_swap1.col == col) && (slot_swap1.row <= row+seq-1) && (slot_swap1.row >= row)) 
                        {
                            if (mat->jewels[slot_swap1.row][col].new_power == NONE){
                                mat->jewels[slot_swap1.row][col].new_type = mat->jewels[slot_swap1.row][col].type;
                                mat->jewels[slot_swap1.row][col].new_power = SQUARE;
                            }
                            else{
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row+seq-1][col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                        else if ((slot_swap2.col == col) && (slot_swap2.row <= row+seq-1) && (slot_swap2.row >= row)) 
                        {
                            if (mat->jewels[slot_swap2.row][col].new_power == NONE){
                                mat->jewels[slot_swap2.row][col].new_type = mat->jewels[slot_swap2.row][col].type;
                                mat->jewels[slot_swap2.row][col].new_power = SQUARE;
                            }
                            else{
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row+seq-1][col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                        else{
                            if (mat->jewels[row+1][col].new_power == NONE){
                            //coloca powerup no meio da sequencia
                                mat->jewels[row+1][col].new_type = mat->jewels[row+1][col].type;
                                mat->jewels[row+1][col].new_power = SQUARE;
                            }
                            else{
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row+seq-1][col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                        increase_score(mat, seq);
                    }
                    else if (seq >= 5)
                    {
                        if ((slot_swap1.col == col) && (slot_swap1.row <= row+seq-1) && (slot_swap1.row >= row)) 
                        {
                            if (mat->jewels[slot_swap1.row][col].new_power == NONE){
                                mat->jewels[slot_swap1.row][col].new_type = WHITE;
                                mat->jewels[slot_swap1.row][col].new_power = DIAMOND;
                            }
                            else{
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = WHITE;
                                mat->jewels[row+seq-1][col].new_power = DIAMOND;
                            }
                        }
                        else if ((slot_swap2.col == col) && (slot_swap2.row <= row+seq-1) && (slot_swap2.row >= row)) 
                        {
                            if (mat->jewels[slot_swap2.row][col].new_power == NONE){
                                mat->jewels[slot_swap2.row][col].new_type = WHITE;
                                mat->jewels[slot_swap2.row][col].new_power = DIAMOND;
                            }
                            else{
                            //coloca powerup no inicio da sequencia
                                mat->jewels[row+seq-1][col].new_type = WHITE;
                                mat->jewels[row+seq-1][col].new_power = DIAMOND;
                            }
                        }
                        else{
                            if (mat->jewels[row+2][col].new_power == NONE){
                            //coloca powerup no meio da sequencia
                                mat->jewels[row+2][col].new_type = WHITE;
                                mat->jewels[row+2][col].new_power = DIAMOND;
                            }
                            else{
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

    if (match){
        destroy_jewels(mat);
        create_new_powers(mat);
    }

    return match;
}

j_type get_new_type(int avail_jewels){
    return (rand()%avail_jewels);
}

/*sets empty jewels new types and sets their downward motion*/
void set_falling(jmat* mat)
{
    int i, j;
    j_type aux;

    for (i = ROW_QT-1; i >= 0; i--){
        for (j = 0; j < COL_QT; j++)
        {
            //count = ((i%ROW_QT)/i)*mat->jewels[i - 1][j].lower;
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

int test_possible_col_match(jmat* mat, int row, int col){
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

int test_possible_row_match(jmat* mat, int row, int col){
    int last_col = COL_QT - 1;

    if (col < last_col){
        j_type Row_PrevCol = mat->jewels[row][col-1].type;
        j_type Row_Col = mat->jewels[row][col].type;
        j_type Row_NextCol = mat->jewels[row][col+1].type;

        if (Row_PrevCol == DIAMOND)
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
            if ((Row_NextCol == DIAMOND) || (mat->jewels[row][col+2].type == DIAMOND))
                return 1;
            else if (((Row_PrevCol == Row_NextCol) && (Row_NextCol == mat->jewels[row][col+2].type)) ||
                    ((Row_PrevCol == Row_Col) && (Row_Col == mat->jewels[row][col+2].type)))
                return 1;
        }
    }

    return 0;
}

int test_end_game(jmat* mat){
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

void swap_jewels_types(jewel* j1, jewel* j2)//, float x_speed, float y_speed)
{
    //j1->vel.x = x_speed;
    //j1->vel.y = y_speed;
    //j1->current.x = j2->proper.x;
    //j1->current.y = j2->proper.y;

    //j2->vel.x = -x_speed;
    //j2->vel.y = -y_speed;
    //j2->current.x = j1->proper.x;
    //j2->current.y = j1->proper.y;

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
int update_all_jewels(jmat *mat)
{
    int moving = 0;

    for (int row = 0; row < ROW_QT; row ++)
        for (int col = 0; col < COL_QT; col ++)
            moving = ( update_jewel(&(mat->jewels[row][col])) || moving );

    return moving;
}


int register_user_input(ALLEGRO_EVENT* event, jmat* mat, int* selected)
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

        if (!*selected)
        {
            //first jewel to be swapped is now selected
            *selected = 1;
            mat->swap1 = &mat->jewels[clicked_slot.row][clicked_slot.col];
        }
        else
        {
            *selected = 0;
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
        *selected = 0; //if click outside matrix, cancel swapping

    return 0;
}

int test_row(jmat *mat, int row){
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

int test_col(jmat *mat, int col){
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

int test_swap(jmat *mat){
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

int initialize_jewel_structure(jmat *mat){

    if ( !(mat->jewels = allocate_jewel_matrix(ROW_QT, COL_QT)) )
        return 0;

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
    mat->score = 0;
    mat->swap1 = NULL;
    mat->swap2 = NULL;

    return 1;
}

void sort_jewels(jmat* mat,int swap_num)
{
    int position;
    vec2 rowcol1;
    vec2 rowcol2;
    for (int i=0; i<swap_num; i++)
    {
        position = rand()%64;
        rowcol1.row = (int)(position/8);
        rowcol1.col = (int)(position%8);

        position = rand()%64;
        rowcol2.row = (int)(position/8);
        rowcol2.col = (int)(position%8);

        swap_jewels_types(&(mat->jewels[rowcol1.row][rowcol1.col]), &(mat->jewels[rowcol2.row][rowcol2.col]));
    }

}

int min(int a, int b)
{
    if (a<b)
        return a;
    return b;
}


int main()
{
    jmat mat;
    mat.pos.x = (SC_W - COL_QT*JEWEL_SIZE)/2;
    mat.pos.y = (SC_H - ROW_QT*JEWEL_SIZE)/2;

    mat.swap1 = NULL;
    mat.swap2 = NULL;

    //al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    //al_set_new_display_flags(ALLEGRO_RESIZABLE);
    //al_set_new_display_flags(ALLEGRO_MAXIMIZED);

    if ( !initialize_jewel_structure(&mat) )
        fprintf(stdin,"erro ao alocar matriz de joias\n");


    //************************************************************//
    //load arquivo de score
    FILE* score_file;

    //le arquivo e atualiza best_score
    score_file = fopen("./resources/files/score_file.txt","r"); 
    must_init(score_file, "arquivo de score");
    int best_score = 0;
    fread(&best_score, sizeof(int), 1, score_file);
    fclose(score_file);

    //************************************************************//

    must_init(al_init_image_addon(),"addon de imagem");
    must_init(al_init(), "allegro");
    must_init(al_init_font_addon(), "addon de fonte");
    must_init(al_init_ttf_addon(), "addon ttf");
    must_init(al_init_primitives_addon(), "addon de primitivas");

    ALLEGRO_DISPLAY* disp = al_create_display(SC_W, SC_H);
    must_init(disp, "display");
    al_set_window_title(disp, "Quende crash");

    //************************************************************//
    //load input structures
    must_init(al_install_keyboard(), "teclado");
    must_init(al_install_mouse(), "mouse");
    must_init(al_set_system_mouse_cursor(disp, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT), "mouse");
    //************************************************************//

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    //ALLEGRO_FONT* score_font = al_create_builtin_font();
    ALLEGRO_FONT* score_font = al_load_ttf_font("./resources/files/CenturyGothicFett.ttf", 26, 0);
    must_init(score_font, "fonte do score");
    ALLEGRO_FONT* game_font = al_load_ttf_font("./resources/files/CenturyGothicFett.ttf", 35, 0);
    must_init(game_font, "fonte do jogo");


    //************************************************************//
    //load sound files
    ALLEGRO_SAMPLE *bg_sound = NULL; 
    ALLEGRO_SAMPLE *swap_sound = NULL; 
    ALLEGRO_SAMPLE *pause_sound = NULL; 
    ALLEGRO_SAMPLE *destroy_sound = NULL; 
    ALLEGRO_SAMPLE *treasure_sound = NULL; 
    ALLEGRO_SAMPLE *next_level_sound = NULL; 
    ALLEGRO_SAMPLE_INSTANCE *bg_sound_inst = NULL; 
    ALLEGRO_SAMPLE_INSTANCE *swap_sound_inst = NULL; 
    ALLEGRO_SAMPLE_INSTANCE *pause_sound_inst = NULL; 
    ALLEGRO_SAMPLE_INSTANCE *destroy_sound_inst = NULL; 
    ALLEGRO_SAMPLE_INSTANCE *treasure_sound_inst = NULL; 
    ALLEGRO_SAMPLE_INSTANCE *next_level_sound_inst = NULL; 
    al_init();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(1);

    bg_sound = al_load_sample("./resources/audio/Howls_Moving_Castle.ogg");    	
    swap_sound = al_load_sample("./resources/audio/swap.ogg");    	
    pause_sound = al_load_sample("./resources/audio/pause.ogg");    	
    destroy_sound = al_load_sample("./resources/audio/destroy.ogg");    	
    treasure_sound = al_load_sample("./resources/audio/treasure.ogg");    	
    next_level_sound = al_load_sample("./resources/audio/next_level.ogg");    	

    must_init(bg_sound,"musica de fundo");
    must_init(swap_sound,"musica de swap");
    must_init(pause_sound,"som de pausa");
    must_init(destroy_sound,"som de destruicao");
    must_init(treasure_sound,"som de easteregg");
    must_init(next_level_sound,"musica de proximo nivel");

    bg_sound_inst = al_create_sample_instance(bg_sound);
    swap_sound_inst = al_create_sample_instance(swap_sound);
    pause_sound_inst = al_create_sample_instance(pause_sound);
    destroy_sound_inst = al_create_sample_instance(destroy_sound);
    treasure_sound_inst = al_create_sample_instance(treasure_sound);
    next_level_sound_inst = al_create_sample_instance(next_level_sound);

    al_attach_sample_instance_to_mixer(bg_sound_inst, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(swap_sound_inst, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(pause_sound_inst, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(destroy_sound_inst, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(treasure_sound_inst, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(next_level_sound_inst, al_get_default_mixer());

    int bg_sound_pos = 0;
    int next_level_sound_pos = 0;
    
    //setagem de opcoes das instancias
    al_set_sample_instance_playmode(bg_sound_inst, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(bg_sound_inst, 0.6);
    al_play_sample_instance(bg_sound_inst);

    al_set_sample_instance_playmode(treasure_sound_inst, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(treasure_sound_inst, 0.7);

    al_set_sample_instance_speed(destroy_sound_inst, 0.6);
    al_set_sample_instance_gain(destroy_sound_inst, 0.7);

    al_set_sample_instance_speed(swap_sound_inst, 0.7);
    al_set_sample_instance_gain(swap_sound_inst, 0.7);

    al_set_sample_instance_speed(next_level_sound_inst, 0.9);
    al_set_sample_instance_gain(next_level_sound_inst, 0.6);

    //************************************************************//



    //************************************************************//
    //load game images
    ALLEGRO_BITMAP* jewel_image[7];
    jewel_image[BLUE] = al_load_bitmap("./resources/sprites/blue.png");
    jewel_image[RED] = al_load_bitmap("./resources/sprites/red.png");
    jewel_image[YELLOW] = al_load_bitmap("./resources/sprites/yellow.png");
    jewel_image[GREEN] = al_load_bitmap("./resources/sprites/green.png");
    jewel_image[GREY] = al_load_bitmap("./resources/sprites/grey.png");
    jewel_image[PURPLE] = al_load_bitmap("./resources/sprites/purple.png");
    jewel_image[WHITE] = al_load_bitmap("./resources/sprites/white.png");
    //test loaded images
    for (int i = 0; i < 7; i++)
        if (!jewel_image[i]){
            fprintf(stderr,"erro ao carregar imagem de joia\n");
            exit(1);
        }
    ALLEGRO_BITMAP* jewel_star_image[6];
    jewel_star_image[BLUE] = al_load_bitmap("./resources/sprites/blue_star.png");
    jewel_star_image[RED] = al_load_bitmap("./resources/sprites/red_star.png");
    jewel_star_image[YELLOW] = al_load_bitmap("./resources/sprites/yellow_star.png");
    jewel_star_image[GREEN] = al_load_bitmap("./resources/sprites/green_star.png");
    jewel_star_image[GREY] = al_load_bitmap("./resources/sprites/grey_star.png");
    jewel_star_image[PURPLE] = al_load_bitmap("./resources/sprites/purple_star.png");
    //test loaded images
    for (int i = 0; i < 6; i++)
        if (!jewel_star_image[i]){
            fprintf(stderr,"erro ao carregar imagem de star power\n");
            exit(1);
        }
    ALLEGRO_BITMAP* jewel_square_image[6];
    jewel_square_image[BLUE] = al_load_bitmap("./resources/sprites/blue_square.png");
    jewel_square_image[RED] = al_load_bitmap("./resources/sprites/red_square.png");
    jewel_square_image[YELLOW] = al_load_bitmap("./resources/sprites/yellow_square.png");
    jewel_square_image[GREEN] = al_load_bitmap("./resources/sprites/green_square.png");
    jewel_square_image[GREY] = al_load_bitmap("./resources/sprites/grey_square.png");
    jewel_square_image[PURPLE] = al_load_bitmap("./resources/sprites/purple_square.png");
    //test loaded images
    for (int i = 0; i < 6; i++)
        if (!jewel_square_image[i]){
            fprintf(stderr,"erro ao carregar imagem de square power\n");
            exit(1);
        }

    ALLEGRO_BITMAP* load_screen;
    load_screen = al_load_bitmap("./resources/sprites/load_screen.png");
    must_init(load_screen,"imagem de loading");

    ALLEGRO_BITMAP* transparent_screen;
    transparent_screen = al_load_bitmap("./resources/sprites/transparent_screen.png");
    must_init(transparent_screen,"imagem de fim de jogo");

    ALLEGRO_BITMAP* bg_image;
    bg_image = al_load_bitmap("./resources/sprites/bg_image.png");
    must_init(bg_image,"imagem de background");

    ALLEGRO_BITMAP* matrix_image;
    matrix_image = al_load_bitmap("./resources/sprites/matriz_bg.png");
    must_init(matrix_image,"imagem de background da matriz");

    ALLEGRO_BITMAP* help_image;
    help_image = al_load_bitmap("./resources/sprites/help_image.png");
    must_init(matrix_image,"imagem da help page");
    //************************************************************//

    al_start_timer(timer);
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    ALLEGRO_EVENT event;
    int cont = 0;
    int last_state = 0;

    int moving;
    vec2 vel;
    int level = 1;
    int selected = 0;
    int player_swap = 0;
    int init = 0;
    int state = JEWEL;
    int new_lev_frames = 0; //amount of frames spent on new level state
    int next_level_score = FIRST_SCORE_GOAL;
    int framerate_divisor = 0;
    int close_game = 0;
    int mistakes = 0;

    int render = 0;
    while (1){
        al_wait_for_event(queue, &event);

        if(event.type == ALLEGRO_EVENT_TIMER)
            render = 1;

        //if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        //    break;

        //**************************************//
        //state independent keyboard input
        if (event.type == ALLEGRO_EVENT_KEY_DOWN){
            switch (event.keyboard.keycode)
            {
                case ALLEGRO_KEY_ESCAPE:
                    //close_game(mat);
                    close_game = 1;
                break;
                case ALLEGRO_KEY_F1: //fallthrough
                case ALLEGRO_KEY_H:
                    if (state == HELP_PAGE)
                    {//quit help page
                        state = last_state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(pause_sound_inst))
                            al_stop_sample_instance(pause_sound_inst);
                        al_play_sample_instance(pause_sound_inst);

                        //sets audio rolling
                        al_play_sample_instance(bg_sound_inst);
                        al_set_sample_instance_position(bg_sound_inst, bg_sound_pos);
                        if (last_state == NEW_LEVEL){
                            al_play_sample_instance(next_level_sound_inst);
                            al_set_sample_instance_position(next_level_sound_inst, next_level_sound_pos);
                        }

                        last_state = HELP_PAGE;
                    }
                    else
                    {//enter help page
                        last_state = state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(pause_sound_inst))
                            al_stop_sample_instance(pause_sound_inst);
                        al_play_sample_instance(pause_sound_inst);

                        //stops easteregg song
                        al_stop_sample_instance(treasure_sound_inst);

                        //saves audio position and stops it
                        bg_sound_pos = al_get_sample_instance_position(bg_sound_inst);
                        al_stop_sample_instance(bg_sound_inst);
                        if (last_state == NEW_LEVEL){
                            next_level_sound_pos = al_get_sample_instance_position(next_level_sound_inst);
                            al_stop_sample_instance(next_level_sound_inst);
                        }
                        state = HELP_PAGE;
                    }
                break;
                case ALLEGRO_KEY_SPACE:
                    if (state == PAUSE)
                    {//unpause game
                        state = last_state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(pause_sound_inst))
                            al_stop_sample_instance(pause_sound_inst);
                        al_play_sample_instance(pause_sound_inst);

                        //sets audio rolling
                        al_play_sample_instance(bg_sound_inst);
                        al_set_sample_instance_position(bg_sound_inst, bg_sound_pos);
                        if (last_state == NEW_LEVEL){
                            al_play_sample_instance(next_level_sound_inst);
                            al_set_sample_instance_position(next_level_sound_inst, next_level_sound_pos);
                        }
                        last_state = PAUSE;
                    }
                    else if (state != HELP_PAGE)
                    {//pause game
                        last_state = state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(pause_sound_inst))
                            al_stop_sample_instance(pause_sound_inst);
                        al_play_sample_instance(pause_sound_inst);

                        //stops easteregg song
                        al_stop_sample_instance(treasure_sound_inst);
                         
                        //saves audio position and stops it
                        bg_sound_pos = al_get_sample_instance_position(bg_sound_inst);
                        al_stop_sample_instance(bg_sound_inst);
                        if (last_state == NEW_LEVEL){
                            next_level_sound_pos = al_get_sample_instance_position(next_level_sound_inst);
                            al_stop_sample_instance(next_level_sound_inst);
                        }

                        state = PAUSE;
                    }
                break;
                default:
                break;
            }
        }
        //**************************************//

        if (close_game)
            break;

        //**************************************//
        //game logic
        switch(state)
        {
            case JEWEL:

                if (mistakes == 5){
                    //play easteregg
                    mistakes = 0;
                    al_stop_sample_instance(bg_sound_inst);
                    al_play_sample_instance(treasure_sound_inst);
                }


                if ( set_to_destroy_matched_jewels(&mat) ){
                    if (al_get_sample_instance_playing(destroy_sound_inst))
                        al_stop_sample_instance(destroy_sound_inst);
                    al_play_sample_instance(destroy_sound_inst);
                    
                    //atualiza arquivo com best score
                    if (mat.score > best_score){
                        score_file = fopen("./resources/files/score_file.txt","w"); 
                        must_init(score_file, "arquivo de score");
                        fwrite(&mat.score, sizeof(int), 1, score_file);
                        fclose(score_file);
                    }

                    set_falling(&mat); //sets jewels downward motion and creates new jewels
                    state = DROP;
                }
                else{

                    if ( test_end_game(&mat) )
                        state = END_GAME; 
                    else{

                        if ( mat.score >= next_level_score  ){
                            al_play_sample_instance(next_level_sound_inst);

                            level++;
                            mat.available_jewels = min((mat.available_jewels+1), MAX_AVAILABLE_JEWELS);
                            state = NEW_LEVEL;
                            next_level_score += 2*next_level_score;
                        }
                        else{
                            init = 1; //set when game is ready for input
                            state = INPUT;
                        }
                    }

                }
                break;
            case INPUT:
                //process input
                if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
                    if ( register_user_input(&event, &mat, &selected) ){
                        if (al_get_sample_instance_playing(swap_sound_inst))
                            al_stop_sample_instance(swap_sound_inst);
                        al_play_sample_instance(swap_sound_inst);
                        state = SWAP;
                    }
                break;
            case SWAP:
                if(event.type == ALLEGRO_EVENT_TIMER)
                { //make it frame rate consistent
                    update_jewel(mat.swap1);
                    moving = update_jewel(mat.swap2);

                    if (!moving){
                        //keep in mind swap1 must be initialized
                        if ( mat.swap2 && ((mat.swap1->power == DIAMOND) || (mat.swap2->power == DIAMOND)) ){
                            if (mat.swap1->power == DIAMOND){
                                vec2 rowcol = get_rowcol(mat.swap1->proper.x, mat.swap1->proper.y, &mat);
                                destroy_jewel(&mat, rowcol.row, rowcol.col, mat.swap2->type);
                            }
                            else{
                                vec2 rowcol = get_rowcol(mat.swap2->proper.x, mat.swap2->proper.y, &mat);
                                destroy_jewel(&mat, rowcol.row, rowcol.col, mat.swap1->type);
                            }
                            set_falling(&mat); //sets jewels downward motion and creates new jewels
                            state = DROP;
                        }
                        else if ( test_swap(&mat) ){
                            mistakes = 0;
                            state = JEWEL;
                        }
                        else{
                            mistakes++;
                            //swap jewels
                            swap_jewels(mat.swap1, mat.swap2, mat.swap1->vel.x, mat.swap1->vel.y);
                            state = DROP; //will update jewels unswapping;
                        }
                    }

                }
                break;
                case DROP:
                    if(event.type == ALLEGRO_EVENT_TIMER){
                            moving = update_all_jewels(&mat);

                            if (!moving)
                                state = JEWEL;
                    }
                break;
                case NEW_LEVEL:
                    if (event.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue))
                    {
                        if (framerate_divisor == NEW_LEVEL_SLOW_DOWN){ 
                            //in every frame_divisor amount of frames:
                            framerate_divisor = 0;
                            sort_jewels(&mat, SORTED_PER_FRAME);
                            new_lev_frames++;
                            if (new_lev_frames == NEW_LEVEL_TIMER){
                                new_lev_frames = 0;
                                state  = JEWEL;
                            }

                        }
                        else
                            framerate_divisor++;
                    }
                break;
                case HELP_PAGE:
                break;
                case END_GAME:
                break;
            default:
                break;
        }
        //**************************************//


        //**************************************//
        //render game
        if(render && al_is_event_queue_empty(queue))
        {
            //redraw_frame();
            if (state == HELP_PAGE)
            {
                al_draw_bitmap(help_image, 0, 0, 0);
                al_flip_display();
            }
            else{
                //al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_bitmap(matrix_image,0,0, 0);


                //if one jewel is selected, draw a hint around it
                if (selected) 
                    al_draw_filled_rounded_rectangle(mat.swap1->proper.x, mat.swap1->proper.y,
                            mat.swap1->proper.x+JEWEL_SIZE, mat.swap1->proper.y+JEWEL_SIZE, 15, 15, al_map_rgba(35,35,60, 100));

                //draw jewel
                for (int i = 0; i< ROW_QT; i++)
                    for (int j = 0; j < COL_QT; j++)
                        if (mat.jewels[i][j].type != EMPTY){
                            if (mat.jewels[i][j].power == NONE)
                                al_draw_bitmap(jewel_image[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                            else if (mat.jewels[i][j].power == STAR){
                                al_draw_bitmap(jewel_star_image[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                            }
                            else if (mat.jewels[i][j].power == SQUARE)
                                al_draw_bitmap(jewel_square_image[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                            else if (mat.jewels[i][j].power == DIAMOND)
                                al_draw_bitmap(jewel_image[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                        }

                //first selected to swap is drawn above
                if (mat.swap2)
                    if (mat.swap2->type != EMPTY ){
                        if (mat.swap2->power == NONE)
                            al_draw_bitmap(jewel_image[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                        else if (mat.swap2->power == STAR)
                            al_draw_bitmap(jewel_star_image[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                        else if (mat.swap2->power == SQUARE)
                            al_draw_bitmap(jewel_square_image[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                        else if (mat.swap2->power == DIAMOND)
                            al_draw_bitmap(jewel_image[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                    }

                //draw upper frame
                //al_draw_filled_rectangle(mat.pos.x, mat.pos.y-200,
                //        mat.pos.x+(COL_QT*JEWEL_SIZE), mat.pos.y, al_map_rgb(0,0,0) );
                al_draw_bitmap(bg_image,0,0,0);


                //**************************************************************//
                //draw UI
                char score_text[19];
                sprintf(score_text, "%09d/%09d", mat.score, next_level_score);
                al_draw_text(score_font, al_map_rgb(255,255,255), 80, 25, 0, score_text);

                sprintf(score_text, "BEST SCORE: %09d", best_score);
                al_draw_text(score_font, al_map_rgb(255,255,255), SC_W-365, 25, 0, score_text);

                sprintf(score_text, "LEVEL: %02d", level);
                al_draw_text(score_font, al_map_rgb(255,255,255), (int)(SC_W/2)-50, 25, 0, score_text);
                //**************************************************************//

                //draw loading screen
                //if (!init){
                //    al_draw_bitmap(transparent_screen, 0, 0, 0);
                //    //al_draw_bitmap(load_screen, (int)(SC_W/2)-100, (int)(SC_H/2)-20, 0);
                //}
                if (state == NEW_LEVEL){
                    //draw_new_level_screen(&game_images);
                    al_draw_bitmap(transparent_screen, 0, 0, 0);
                    al_draw_text(game_font, al_map_rgb(255,255,255), (int)(SC_W/2)-90, SC_H-80, 0, "NEW LEVEL!");
                }
                else if(state == PAUSE){
                    if (last_state == NEW_LEVEL){
                        //draw_new_level_screen(&game_images);
                        al_draw_bitmap(transparent_screen, 0, 0, 0);
                        al_draw_text(game_font, al_map_rgb(255,255,255), (int)(SC_W/2)-90, SC_H-80, 0, "NEW LEVEL!");
                    }
                    al_draw_bitmap(transparent_screen, 0, 0, 0);
                    al_draw_text(game_font, al_map_rgb(255,255,255), (int)(SC_W/2)-53, (int)(SC_H/2)-20, 0, "PAUSE");
                }
                else if (state == END_GAME){
                    //draw end game screen
                    al_draw_bitmap(transparent_screen, 0, 0, 0);
                    al_draw_text(game_font, al_map_rgb(255,255,255), (int)(SC_W/2)-90, (int)(SC_H/2)-20, 0, "YOU LOST");
                }
                
                al_flip_display();
            }
            render = 0;
        }
        //**************************************//
    }

    //**************************************//
    //deallocate structures
    for (int i = 0; i < 7; i++){
        al_destroy_bitmap(jewel_image[i]);
    }
    for (int i = 0; i < 6; i++){
        al_destroy_bitmap(jewel_star_image[i]);
        al_destroy_bitmap(jewel_square_image[i]);
    }

    al_destroy_bitmap(load_screen);
    al_destroy_bitmap(transparent_screen);
    al_destroy_bitmap(bg_image);
    al_destroy_bitmap(matrix_image);
    al_destroy_bitmap(help_image);
    free(mat.jewels);

    al_destroy_font(score_font);
    al_destroy_font(game_font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    al_destroy_sample(bg_sound);
    al_destroy_sample(swap_sound);
    al_destroy_sample(pause_sound);
    al_destroy_sample(destroy_sound);
    al_destroy_sample(treasure_sound);
    al_destroy_sample(next_level_sound);

    al_destroy_sample_instance(bg_sound_inst);
    al_destroy_sample_instance(swap_sound_inst);
    al_destroy_sample_instance(pause_sound_inst);
    al_destroy_sample_instance(destroy_sound_inst);
    al_destroy_sample_instance(treasure_sound_inst);
    al_destroy_sample_instance(next_level_sound_inst);
    //**************************************//

    return 0;
}
