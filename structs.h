#ifndef __STRUCTS__
#define __STRUCTS__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/display.h>
#include <allegro5/events.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>


#define BG_AUDIO        0
#define SWAP_AUDIO      1
#define PAUSE_AUDIO     2
#define DESTROY_AUDIO   3
#define TREASURE_AUDIO  4
#define NEW_LEVEL_AUDIO 5


#define BG_IMAGE            0
#define MAT_IMAGE           1
#define HELP_IMAGE          2
#define TRANSPARENT_IMAGE   3


#define SCORE_FONT 0
#define GAME_FONT  1


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
    ALLEGRO_BITMAP* sprite[7];
    ALLEGRO_BITMAP* star_sprite[6];
    ALLEGRO_BITMAP* square_sprite[6];
    ALLEGRO_BITMAP* screen[4];
} game_images;

typedef struct game_audio{
    ALLEGRO_SAMPLE* sample[6];
    ALLEGRO_SAMPLE_INSTANCE* sample_inst[6];
    unsigned int inst_pos[6];
} game_audio;


typedef struct game_struct{
    jewel **jewels;         //jewel matrix
    jewel *swap1, *swap2;   //last two swaped jewels
    vec2 pos;               //xy position of jewel matrix
    int available_jewels;   //num of jewels available in current level
                            
    int score;

    game_images image;
    game_audio audio;

    ALLEGRO_DISPLAY* disp;
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT_QUEUE* queue;
    ALLEGRO_FONT* font[2];
    ALLEGRO_EVENT event;
} game_struct;

#endif
