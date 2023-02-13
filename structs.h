#ifndef __STRUCTS__
#define __STRUCTS__

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

typedef struct game_struct{
    jewel **jewels;         //jewel matrix
    jewel *swap1, *swap2;   //last two swaped jewels
    vec2 pos;               //xy position of jewel matrix
    int score;
    int available_jewels;   //num of jewels available in current level
    game_images image;
    game_audio audio;
} game_struct;

#endif
