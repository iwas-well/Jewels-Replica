#include <allegro5/color.h>
#include <allegro5/display.h>
#include <allegro5/events.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

//todo:
//  3 power ups 
//  score
//  best score file
//  shuffle when end level
//  multiple levels
//  background image
//  sound
//  easteregg
//  art

//game configuration
#define SC_W        1300 //screen width
#define SC_H        750  //screen height
#define ROW_QT      8    //quantity of lines on the game matrix
#define COL_QT      8    //quantity of columns on the game matrix
#define JEWEL_SIZE  65   //lenght of jewel slot
#define VELOCITY    3    //jewel movement velocity
                         
//game states
#define INPUT       0
#define JEWEL       1
#define DROP        2
#define SWAP        3
#define END_GAME    4

//status
#define NONE            0
#define DESTROY         1
#define DESTROY_STAR    2
#define DESTROY_SQUARE  3

//powerup
#define NONE        0
#define STAR        1
#define SQUARE      2

#define DIAMOND 9
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

typedef struct jmat{
    jewel **jewels;         //jewel matrix
    jewel *swap1, *swap2;   //last two swaped jewels
    vec2 pos;               //xy position of jewel matrix
    int score;
} jmat;

void destroy_jewel(jmat* mat, int row, int col);

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

void star_destroyer(jmat* mat, int row, int col)
{
    //destroi linha
    for (int j = 0; j < COL_QT; j++)
        destroy_jewel(mat,row,j);

    //destroi coluna
    for (int i = 0; i < ROW_QT; i++)
        destroy_jewel(mat, i, col);
}

void diamond_destroyer(jmat *mat, j_type type)
{
    int i, j;

    for (i = 0; i < ROW_QT; i++)
        for (j = 0; j < COL_QT; j++)
            if (mat->jewels[i][j].type == type)
                destroy_jewel(mat, i, j);
}

void square_destroyer(jmat* mat, int row, int col)
{
    int aux_row;
    for (int i = 0; i < 3; i++) {
        aux_row = (row-1)+i;

        if ((aux_row < ROW_QT) && ( aux_row >= 0)){

            //destroi jewels na linha de acima
            if ((col-1) >= 0)
                destroy_jewel(mat, (row-1)+i, col-1);

            //destroi jewels da linha do meio
            destroy_jewel(mat, (row-1)+i, col);

            //destroi jewels na linha de baixo
            if (col+1 < COL_QT)
                destroy_jewel(mat, (row-1)+i, col+1);
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
                destroy_jewel(mat, row, col);
}

void destroy_jewel(jmat* mat, int row, int col)
{
    int aux_power;
    aux_power = mat->jewels[row][col].power;

    mat->jewels[row][col].type = EMPTY;
    mat->jewels[row][col].status = NONE;
    mat->jewels[row][col].power = NONE;

    if (aux_power == STAR)
        star_destroyer(mat, row, col);
    else if (aux_power == SQUARE)
        square_destroyer(mat, row, col);

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

                if (mat->swap2){
                    vec2 rowcol = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    if (seq == 4){
                        if ((rowcol.col <= col+(seq-1)) && (rowcol.col >= col)){ //think its impossible to be otherwise
                            mat->jewels[row][rowcol.col].new_type = mat->jewels[row][rowcol.col].type;
                            mat->jewels[row][rowcol.col].new_power = SQUARE;
                        }
                    }
                    else if (seq == 5){
                        if ((rowcol.col <= col+(seq-1)) && (rowcol.col >= col)){ //think its impossible to be otherwise
                            mat->jewels[row][rowcol.col].new_type = WHITE;
                            mat->jewels[row][rowcol.col].new_power = DIAMOND;
                        }
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
                        mat->jewels[row+k][col].new_type = mat->jewels[row][col].type;
                        mat->jewels[row+k][col].new_power = STAR;
                    }
                    mat->jewels[row+k][col].status = DESTROY;
                }

                if (mat->swap2)
                {
                    vec2 rowcol = get_rowcol(mat->swap1->current.x, mat->swap1->current.y, mat);
                    if (seq == 4)
                    {
                        if ((rowcol.row <= row+(seq-1)) && (rowcol.row >= row)){ //think its impossible to be otherwise
                            if (mat->jewels[rowcol.row][col].new_power == NONE){
                                mat->jewels[rowcol.row][col].new_type = mat->jewels[row][col].type;
                                mat->jewels[rowcol.row][col].new_power = SQUARE;
                            }
                            else{
                                mat->jewels[row+seq-1][col].new_type = mat->jewels[row][rowcol.col].type;
                                mat->jewels[row+seq-1][col].new_power = SQUARE;
                            }
                        }
                    }
                    else if (seq == 5)
                    {
                        if ((rowcol.row <= row+(seq-1)) && (rowcol.row >= row)){ //think its impossible to be otherwise
                            if (mat->jewels[rowcol.row][col].new_power == NONE){
                                mat->jewels[rowcol.row][col].new_type = WHITE;
                                mat->jewels[rowcol.row][col].new_power = DIAMOND;
                            }
                            else{
                                mat->jewels[row+seq-1][col].new_type = WHITE;
                                mat->jewels[row+seq-1][col].new_power = DIAMOND;
                            }
                        }
                    }
                }
            }//if seq >= 3
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


j_type get_new_type(){
    return (rand()%6);
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
                    j_type new_jewel = get_new_type();
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
        if ((col+2) <= last_col)
            if (((Row_PrevCol == Row_NextCol) && (Row_NextCol == mat->jewels[row][col+2].type)) ||
                ((Row_PrevCol == Row_Col) && (Row_Col == mat->jewels[row][col+2].type)))
                return 1;
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

/*swap jewels 'a' and 'b' types and set their velocity in opposit directions,
 * x_speed y_speed being the x velocity and y velocity of jewel 'a'*/
void swap_jewels(jewel* j1, jewel* j2, float x_speed, float y_speed)
{
    j1->vel.x = x_speed;
    j1->vel.y = y_speed;
    j1->current.x = j2->proper.x;
    j1->current.y = j2->proper.y;

    j2->vel.x = -x_speed;
    j2->vel.y = -y_speed;
    j2->current.x = j1->proper.x;
    j2->current.y = j1->proper.y;

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
            mat->jewels[i][j].type = get_new_type();
            mat->jewels[i][j].new_power = NONE;
            mat->jewels[i][j].new_type = EMPTY;
            mat->jewels[i][j].status = NONE;
            mat->jewels[i][j].lower = 0;
        }
    mat->score = 0;
    mat->swap1 = NULL;
    mat->swap2 = NULL;

    return 1;
}


int main()
{
    jmat mat;
    mat.pos.x = (SC_W - COL_QT*JEWEL_SIZE)/2;
    mat.pos.y = (SC_H - ROW_QT*JEWEL_SIZE)/2;

    mat.swap1 = NULL;
    mat.swap2 = NULL;

    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    al_set_new_display_flags(ALLEGRO_RESIZABLE);
    al_set_new_display_flags(ALLEGRO_MAXIMIZED);

    if ( !initialize_jewel_structure(&mat) )
        fprintf(stdin,"erro ao alocar matriz de joias\n");

    must_init(al_init_image_addon(),"addon de imagem");
    must_init(al_init(), "allegro");
    must_init(al_init_primitives_addon(), "addon de primitivas");

    ALLEGRO_DISPLAY* disp = al_create_display(SC_W, SC_H);
    must_init(disp, "display");
    al_set_window_title(disp, "Quende crash");

    must_init(al_install_keyboard(), "teclado");
    must_init(al_install_mouse(), "mouse");
    must_init(al_set_system_mouse_cursor(disp, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT), "mouse");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    ALLEGRO_FONT* font = al_create_builtin_font();
    must_init(font, "fonte");

    ALLEGRO_BITMAP* jewel_image[6];
    jewel_image[BLUE] = al_load_bitmap("./sprites/blue.png");
    jewel_image[RED] = al_load_bitmap("./sprites/red.png");
    jewel_image[YELLOW] = al_load_bitmap("./sprites/yellow.png");
    jewel_image[GREEN] = al_load_bitmap("./sprites/green.png");
    jewel_image[GREY] = al_load_bitmap("./sprites/white.png");
    jewel_image[PURPLE] = al_load_bitmap("./sprites/purple.png");

    //test loaded images
    for (int i = 0; i < 6; i++)
        if (!jewel_image[i]){
            fprintf(stderr,"erro ao carregar imagem de joia\n");
            exit(1);
        }

    ALLEGRO_BITMAP* jewel_up_image[6];
    jewel_up_image[BLUE] = al_load_bitmap("./sprites/blue_up.png");
    jewel_up_image[RED] = al_load_bitmap("./sprites/red_up.png");
    jewel_up_image[YELLOW] = al_load_bitmap("./sprites/yellow_up.png");
    jewel_up_image[GREEN] = al_load_bitmap("./sprites/green_up.png");
    jewel_up_image[GREY] = al_load_bitmap("./sprites/white_up.png");
    jewel_up_image[PURPLE] = al_load_bitmap("./sprites/purple_up.png");

    //test loaded images
    for (int i = 0; i < 6; i++)
        if (!jewel_up_image[i]){
            fprintf(stderr,"erro ao carregar imagem de power\n");
            exit(1);
        }

    ALLEGRO_BITMAP* load_screen;
    load_screen = al_load_bitmap("./sprites/load_screen.png");
    must_init(load_screen,"imagem de loading");
    ALLEGRO_BITMAP* transparent_screen;
    transparent_screen = al_load_bitmap("./sprites/transparent_screen.png");
    must_init(transparent_screen,"imagem de fim de jogo");

    al_start_timer(timer);
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    ALLEGRO_EVENT event;
    int cont = 0;

    int moving;
    vec2 vel;
    int selected = 0;
    int player_swap = 0;
    int init = 0;
    int state = JEWEL;

    int render = 0;
    while (1){
        al_wait_for_event(queue, &event);

        if(event.type == ALLEGRO_EVENT_TIMER)
            render = 1;
        else if((event.type == ALLEGRO_EVENT_KEY_DOWN) || (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE))
            break;

        switch(state){
            case JEWEL:

                if ( set_to_destroy_matched_jewels(&mat) ){
                //else if ( destroy_matched_jewels(&mat) ){
                    set_falling(&mat); //sets jewels downward motion and creates new jewels
                    state = DROP;
                }
                else{

                    if ( test_end_game(&mat) )
                        state = END_GAME; 
                    else{
                        init = 1; //set when game is ready for input
                        state = INPUT;
                    }
                }
                break;
            case INPUT:
                //process input
                if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
                    if ( register_user_input(&event, &mat, &selected) ){
                        state = SWAP;
                    }
                break;
            case SWAP:
                if(event.type == ALLEGRO_EVENT_TIMER){ //make it frame rate consistent
                    moving = update_jewel(mat.swap1);
                    moving = (update_jewel(mat.swap2) || moving);

                    if (!moving){
                        //keep in mind swap1 must be initialized
                        if ( mat.swap2 && ((mat.swap1->power == DIAMOND) || (mat.swap2->power == DIAMOND)) ){
                            if (mat.swap1->power == DIAMOND){
                                mat.swap1->power = NONE;
                                mat.swap1->type = EMPTY;
                                diamond_destroyer(&mat, mat.swap2->type);
                            }
                            else{
                                mat.swap2->power = NONE;
                                mat.swap2->type = EMPTY;
                                diamond_destroyer(&mat, mat.swap2->type);
                            }
                            set_falling(&mat); //sets jewels downward motion and creates new jewels
                            state = DROP;
                        }
                        else if ( test_swap(&mat) )
                            state = JEWEL;
                        else{
                            //swap jewels
                            swap_jewels(mat.swap1, mat.swap2, mat.swap1->vel.x, mat.swap1->vel.y);
                            state = DROP;
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
                case END_GAME:
                    printf("\rperdeu playboy");
                    fflush(stdout);
                break;
            default:
                break;
        }

        if(render && al_is_event_queue_empty(queue))
        {
            //redraw_frame();
            
            al_clear_to_color(al_map_rgb(0, 0, 0));
           
            //if one jewel is selected, draw a hint around it
            if (selected) 
                al_draw_filled_rounded_rectangle(mat.swap1->proper.x, mat.swap1->proper.y,
                        mat.swap1->proper.x+JEWEL_SIZE, mat.swap1->proper.y+JEWEL_SIZE, 15, 15, al_map_rgb(15,15,15));

            //draw jewel
            for (int i = 0; i< ROW_QT; i++)
                for (int j = 0; j < COL_QT; j++)
                    if (mat.jewels[i][j].type != EMPTY){
                        if (mat.jewels[i][j].power == NONE)
                            al_draw_bitmap(jewel_image[ mat.jewels[i][j].type ],
                                    mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                        else if (mat.jewels[i][j].power == STAR){
                            al_draw_bitmap(jewel_up_image[ mat.jewels[i][j].type ],
                                    mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                        }
                        else if (mat.jewels[i][j].power == SQUARE)
                            al_draw_bitmap(jewel_up_image[ mat.jewels[i][j].type ],
                                    mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                        else if (mat.jewels[i][j].power == DIAMOND)
                            al_draw_filled_rectangle(mat.jewels[i][j].current.x+10, mat.jewels[i][j].current.y+10, mat.jewels[i][j].current.x+JEWEL_SIZE-10,
                                    mat.jewels[i][j].current.y+JEWEL_SIZE-10, al_map_rgb(255,255,255));
                            //al_draw_bitmap(jewel_up_image[ mat.jewels[i][j].type ],
                            //        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                    }

            //first selected to swap is drawn above
            if (state == SWAP && mat.swap2)
                if (mat.swap2->type != EMPTY ){
                    if (mat.swap2->power == NONE)
                        al_draw_bitmap(jewel_image[ mat.swap2->type ],
                                mat.swap2->current.x, mat.swap2->current.y, 0);
                    else if (mat.swap2->power == STAR)
                        al_draw_bitmap(jewel_up_image[ mat.swap2->type ],
                                mat.swap2->current.x, mat.swap2->current.y, 0);
                    else if (mat.swap2->power == SQUARE)
                        al_draw_bitmap(jewel_up_image[ mat.swap2->type ],
                                mat.swap2->current.x, mat.swap2->current.y, 0);
                    else if (mat.swap2->power == DIAMOND)
                        al_draw_filled_rectangle(mat.swap2->current.x+5, mat.swap2->current.y+5, mat.swap2->current.x+JEWEL_SIZE-5,
                                mat.swap2->current.y+JEWEL_SIZE-5, al_map_rgb(255,255,255));
                        //al_draw_bitmap(jewel_up_image[ mat.swap2->type ],
                        //        mat.swap2->current.x, mat.swap2->current.y, 0);
                }

            //draw upper frame
            al_draw_filled_rectangle(mat.pos.x, mat.pos.y-200,
                    mat.pos.x+(COL_QT*JEWEL_SIZE), mat.pos.y, al_map_rgb(0,0,0) );

            //draw loading screen
            if (!init){
                al_clear_to_color(al_map_rgba(0, 0, 0,50));
                //al_draw_bitmap(load_screen, (int)(SC_W/2)-100, (int)(SC_H/2)-20, 0);
            }

            if (state == END_GAME){
                al_draw_bitmap(transparent_screen, 0, 0, 0);
                al_draw_text(font, al_map_rgb(255,255,255), (int)(SC_W/2)-20, (int)(SC_H/2)-20, 0, "You Lost");
            }

            al_flip_display();

            render = 0;
        }
    }

    //deallocate structures
    for (int i = 0; i < 6; i++){
        al_destroy_bitmap(jewel_image[i]);
        al_destroy_bitmap(jewel_up_image[i]);
    }
    al_destroy_bitmap(load_screen);
    al_destroy_bitmap(transparent_screen);
    free(mat.jewels);

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
