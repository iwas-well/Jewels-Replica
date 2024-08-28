#include <stdio.h>
#include <allegro5/color.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/display.h>
#include <allegro5/events.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#include "allegro_dependencies.h"
#include "structs.h"
#include "libgame.h" 

//terminates the program if "test" parameter is 0 (NULL)
//printing message given by the "description" parameter
void must_init(bool test, const char *description)
{
    if(test) return;
    printf("Nao foi possivel inicializar %s\n", description);
    exit(1);
}

void deallocate_allegro_structures(game_struct* game)
{
    for (int i = 0; i < 7; i++)
        al_destroy_bitmap(game->image.sprite[i]);

    for (int i = 0; i < 6; i++){
        al_destroy_bitmap(game->image.star_sprite[i]);
        al_destroy_bitmap(game->image.square_sprite[i]);
    }

    al_destroy_bitmap(game->image.screen[TRANSPARENT_IMAGE]);
    al_destroy_bitmap(game->image.screen[BG_IMAGE]);
    al_destroy_bitmap(game->image.screen[MAT_IMAGE]);
    al_destroy_bitmap(game->image.screen[HELP_IMAGE]);

    al_destroy_font(game->font[SCORE_FONT]);
    al_destroy_font(game->font[GAME_FONT]);
    al_destroy_display(game->disp);
    al_destroy_timer(game->fr_timer);
    al_destroy_timer(game->timer);
    al_destroy_event_queue(game->queue);

    al_destroy_sample(game->audio.sample[BG_AUDIO]);
    al_destroy_sample(game->audio.sample[SWAP_AUDIO]);
    al_destroy_sample(game->audio.sample[PAUSE_AUDIO]);
    al_destroy_sample(game->audio.sample[DESTROY_AUDIO]);
    al_destroy_sample(game->audio.sample[TREASURE_AUDIO]);
    al_destroy_sample(game->audio.sample[NEW_LEVEL_AUDIO]);

    al_destroy_sample_instance(game->audio.sample_inst[BG_AUDIO]);
    al_destroy_sample_instance(game->audio.sample_inst[SWAP_AUDIO]);
    al_destroy_sample_instance(game->audio.sample_inst[PAUSE_AUDIO]);
    al_destroy_sample_instance(game->audio.sample_inst[DESTROY_AUDIO]);
    al_destroy_sample_instance(game->audio.sample_inst[TREASURE_AUDIO]);
    al_destroy_sample_instance(game->audio.sample_inst[NEW_LEVEL_AUDIO]);
}

//loads all audio resources of the game and attaches them to the default mixer
static void load_samples(game_struct *game) {
    game->audio.sample[SWAP_AUDIO] = al_load_sample("./resources/audio/swap.ogg");    	
    game->audio.sample[PAUSE_AUDIO] = al_load_sample("./resources/audio/pause.ogg");    	
    game->audio.sample[DESTROY_AUDIO] = al_load_sample("./resources/audio/destroy.ogg");    	
    game->audio.sample[TREASURE_AUDIO] = al_load_sample("./resources/audio/treasure.ogg");    	
    game->audio.sample[NEW_LEVEL_AUDIO] = al_load_sample("./resources/audio/next_level.ogg");    	
    game->audio.sample[BG_AUDIO] = al_load_sample("./resources/audio/Howls_Moving_Castle.ogg");    	

    must_init(game->audio.sample[PAUSE_AUDIO],"som de pausa");
    must_init(game->audio.sample[BG_AUDIO],"musica de fundo");
    must_init(game->audio.sample[SWAP_AUDIO],"musica de swap");
    must_init(game->audio.sample[DESTROY_AUDIO],"som de destruicao");
    must_init(game->audio.sample[TREASURE_AUDIO],"som de easteregg");
    must_init(game->audio.sample[NEW_LEVEL_AUDIO],"musica de proximo nivel");

    game->audio.sample_inst[BG_AUDIO] = al_create_sample_instance(game->audio.sample[BG_AUDIO]);
    game->audio.sample_inst[SWAP_AUDIO] = al_create_sample_instance(game->audio.sample[SWAP_AUDIO]);
    game->audio.sample_inst[PAUSE_AUDIO] = al_create_sample_instance(game->audio.sample[PAUSE_AUDIO]);
    game->audio.sample_inst[DESTROY_AUDIO] = al_create_sample_instance(game->audio.sample[DESTROY_AUDIO]);
    game->audio.sample_inst[TREASURE_AUDIO] = al_create_sample_instance(game->audio.sample[TREASURE_AUDIO]);
    game->audio.sample_inst[NEW_LEVEL_AUDIO] = al_create_sample_instance(game->audio.sample[NEW_LEVEL_AUDIO]);

    al_attach_sample_instance_to_mixer(game->audio.sample_inst[BG_AUDIO], al_get_default_mixer());
    al_attach_sample_instance_to_mixer(game->audio.sample_inst[SWAP_AUDIO], al_get_default_mixer());
    al_attach_sample_instance_to_mixer(game->audio.sample_inst[PAUSE_AUDIO], al_get_default_mixer());
    al_attach_sample_instance_to_mixer(game->audio.sample_inst[DESTROY_AUDIO], al_get_default_mixer());
    al_attach_sample_instance_to_mixer(game->audio.sample_inst[TREASURE_AUDIO], al_get_default_mixer());
    al_attach_sample_instance_to_mixer(game->audio.sample_inst[NEW_LEVEL_AUDIO], al_get_default_mixer());

}

//loads all game images and sprites
static void load_sprites(game_struct *game) {
    //-----------------------------------
    //load game images
    //load base jewel sprites
    game->image.sprite[BLUE] = al_load_bitmap("./resources/sprites/blue.png");
    game->image.sprite[RED] = al_load_bitmap("./resources/sprites/red.png");
    game->image.sprite[YELLOW] = al_load_bitmap("./resources/sprites/yellow.png");
    game->image.sprite[GREEN] = al_load_bitmap("./resources/sprites/green.png");
    game->image.sprite[GREY] = al_load_bitmap("./resources/sprites/grey.png");
    game->image.sprite[PURPLE] = al_load_bitmap("./resources/sprites/purple.png");
    game->image.sprite[WHITE] = al_load_bitmap("./resources/sprites/white.png");

    //test loaded images
    for (int i = 0; i < 7; i++)
        must_init(game->image.sprite[i], "imagem de star joia");

    //load star powerup sprites
    game->image.star_sprite[BLUE] = al_load_bitmap("./resources/sprites/blue_star.png");
    game->image.star_sprite[RED] = al_load_bitmap("./resources/sprites/red_star.png");
    game->image.star_sprite[YELLOW] = al_load_bitmap("./resources/sprites/yellow_star.png");
    game->image.star_sprite[GREEN] = al_load_bitmap("./resources/sprites/green_star.png");
    game->image.star_sprite[GREY] = al_load_bitmap("./resources/sprites/grey_star.png");
    game->image.star_sprite[PURPLE] = al_load_bitmap("./resources/sprites/purple_star.png");

    //load square powerup sprites
    game->image.square_sprite[BLUE] = al_load_bitmap("./resources/sprites/blue_square.png");
    game->image.square_sprite[RED] = al_load_bitmap("./resources/sprites/red_square.png");
    game->image.square_sprite[YELLOW] = al_load_bitmap("./resources/sprites/yellow_square.png");
    game->image.square_sprite[GREEN] = al_load_bitmap("./resources/sprites/green_square.png");
    game->image.square_sprite[GREY] = al_load_bitmap("./resources/sprites/grey_square.png");
    game->image.square_sprite[PURPLE] = al_load_bitmap("./resources/sprites/purple_square.png");

    //test loaded powerup images
    for (int i = 0; i < 6; i++) {
        must_init(game->image.star_sprite[i], "imagem de star power");
        must_init(game->image.square_sprite[i], "imagem de square power");
    }

    //load screen sprites
    game->image.screen[BG_IMAGE] = al_load_bitmap("./resources/sprites/bg_image.png");
    game->image.screen[MAT_IMAGE] = al_load_bitmap("./resources/sprites/matriz_bg.png");
    game->image.screen[HELP_IMAGE] = al_load_bitmap("./resources/sprites/help_image.png");
    game->image.screen[TRANSPARENT_IMAGE] = al_load_bitmap("./resources/sprites/transparent_screen.png");

    must_init(game->image.screen[BG_IMAGE],"imagem de background");
    must_init(game->image.screen[MAT_IMAGE],"imagem de background da matriz");
    must_init(game->image.screen[HELP_IMAGE],"imagem da help page");
    must_init(game->image.screen[TRANSPARENT_IMAGE],"imagem de fim de jogo");
    //-----------------------------------
}

int initialize_allegro_dependencies(game_struct* game)
{
    //initialize basic funcionalities
    must_init(al_init(), "allegro");
    must_init(al_init_image_addon(),"addon de imagem");
    must_init(al_init_font_addon(), "addon de fonte");
    must_init(al_init_ttf_addon(), "addon ttf");
    must_init(al_init_primitives_addon(), "addon de primitivas");
    must_init(al_install_audio(), "audio");
    must_init(al_init_acodec_addon(), "audio codec");

    //initialize display
    game->disp = al_create_display(SC_W, SC_H);
    must_init(game->disp, "display");
    al_set_window_title(game->disp, "Match 3");

    //load user input structures
    must_init(al_install_keyboard(), "teclado");
    must_init(al_install_mouse(), "mouse");
    must_init(al_set_system_mouse_cursor(game->disp, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT), "mouse");
    
    //load game fonts
    game->font[SCORE_FONT] = al_load_ttf_font("./resources/files/CenturyGothicFett.ttf", 26, 0);
    must_init(game->font[SCORE_FONT], "fonte do score");
    game->font[GAME_FONT] = al_load_ttf_font("./resources/files/CenturyGothicFett.ttf", 35, 0);
    must_init(game->font[GAME_FONT], "fonte do jogo");

    //create game event queue
    game->queue = al_create_event_queue();
    must_init(game->queue, "queue");

    //creates a voice and set it as default mixer
    //(the mixer mixes attached streams into a single buffer)
    al_reserve_samples(1);

    load_samples(game);

    game->audio.inst_pos[BG_AUDIO] = 0;
    game->audio.inst_pos[NEW_LEVEL_AUDIO] = 0;
    
    //-----------------------------------
    //set samples parameters
    al_set_sample_instance_playmode(game->audio.sample_inst[BG_AUDIO], ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(game->audio.sample_inst[BG_AUDIO], 0.6);

    al_set_sample_instance_playmode(game->audio.sample_inst[TREASURE_AUDIO], ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(game->audio.sample_inst[TREASURE_AUDIO], 0.7);

    al_set_sample_instance_speed(game->audio.sample_inst[DESTROY_AUDIO], 0.6);
    al_set_sample_instance_gain(game->audio.sample_inst[DESTROY_AUDIO], 0.7);

    al_set_sample_instance_speed(game->audio.sample_inst[SWAP_AUDIO], 0.7);
    al_set_sample_instance_gain(game->audio.sample_inst[SWAP_AUDIO], 0.7);

    al_set_sample_instance_speed(game->audio.sample_inst[NEW_LEVEL_AUDIO], 0.9);
    al_set_sample_instance_gain(game->audio.sample_inst[NEW_LEVEL_AUDIO], 0.6);
    //-----------------------------------

    //set background music to play 
    al_play_sample_instance(game->audio.sample_inst[BG_AUDIO]);

    load_sprites(game);

    //set game frame rate timer
    game->fr_timer = al_create_timer(1.0/FPS);
    must_init(game->fr_timer, "frame timer");
    al_register_event_source(game->queue, al_get_timer_event_source(game->fr_timer));

    //set in game timer
    game->timer = al_create_timer(0);
    must_init(game->timer, "timer");
    al_register_event_source(game->queue, al_get_timer_event_source(game->timer));

    //start frame rate timer
    al_start_timer(game->fr_timer);

    //register input events
    al_register_event_source(game->queue, al_get_keyboard_event_source());
    al_register_event_source(game->queue, al_get_mouse_event_source());
    al_register_event_source(game->queue, al_get_display_event_source(game->disp));

    return 0;
}
