#include <stdio.h>
#include <stdlib.h>

#include "render_game.h"
#include "structs.h"
#include "allegro_dependencies.h"
#include "libgame.h"
#include "jewels_destruction.h"
#include "jewels_movement.h"

//todo:
//  rock
//  destruction animation

int main()
{
    game_struct mat;

    if ( !initialize_jewel_structure(&mat) )
        fprintf(stdin,"erro ao alocar matriz de joias\n");

    initialize_allegro_dependencies(&mat);

    //************************************************************//
    
    //load arquivo de score
    FILE* score_file;

    //le arquivo e atualiza best_score
    score_file = fopen("./resources/files/score_file.txt","r"); 
    must_init(score_file, "arquivo de score");
    fread(&mat.best_score, sizeof(int), 1, score_file);
    fclose(score_file);

    //************************************************************//

    int moving;
    int count_frames = 0;
    int new_lev_frames = 0; //amount of frames spent on new level state
    int framerate_divisor = 0;
    int mistakes = 0;  //count mistakes for easteregg

    int render = 0;
    while (1){
        al_wait_for_event(mat.queue, &mat.event);

        if(mat.event.type == ALLEGRO_EVENT_TIMER)
            render = 1;

        if(mat.event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;

        //**************************************//
        //state independent keyboard input
        if (mat.event.type == ALLEGRO_EVENT_KEY_DOWN){
            switch (mat.event.keyboard.keycode)
            {
                case ALLEGRO_KEY_F1: //fallthrough
                case ALLEGRO_KEY_H:
                    if (mat.state == HELP_PAGE)
                    {//quit help page
                        mat.state = mat.last_state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //sets audio rolling
                        al_play_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        al_set_sample_instance_position(mat.audio.sample_inst[BG_AUDIO], mat.audio.inst_pos[BG_AUDIO]);
                        if (mat.last_state == NEW_LEVEL){
                            al_play_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_set_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO], mat.audio.inst_pos[NEW_LEVEL_AUDIO]);
                        }

                        mat.last_state = HELP_PAGE;
                    }
                    else
                    {//enter help page
                        mat.last_state = mat.state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //stops easteregg song
                        al_stop_sample_instance(mat.audio.sample_inst[TREASURE_AUDIO]);

                        //saves audio position and stops it
                        mat.audio.inst_pos[BG_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[BG_AUDIO]);
                        al_stop_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        if (mat.last_state == NEW_LEVEL){
                            mat.audio.inst_pos[NEW_LEVEL_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_stop_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                        }
                        mat.state = HELP_PAGE;
                    }
                break;
                case ALLEGRO_KEY_ESCAPE: //fallthrough
                case ALLEGRO_KEY_SPACE:
                    if (mat.state == PAUSE)
                    {//unpause game
                        mat.state = mat.last_state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //sets audio rolling
                        al_play_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        al_set_sample_instance_position(mat.audio.sample_inst[BG_AUDIO], mat.audio.inst_pos[BG_AUDIO]);
                        if (mat.last_state == NEW_LEVEL){
                            al_play_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_set_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO], mat.audio.inst_pos[NEW_LEVEL_AUDIO]);
                        }
                        mat.last_state = PAUSE;
                    }
                    else if ((mat.state != HELP_PAGE) && (mat.state != END_GAME))
                    {//pause game
                        mat.last_state = mat.state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //stops easteregg song
                        al_stop_sample_instance(mat.audio.sample_inst[TREASURE_AUDIO]);
                         
                        //saves audio position and stops it
                        mat.audio.inst_pos[BG_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[BG_AUDIO]);
                        al_stop_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        if (mat.last_state == NEW_LEVEL){
                            mat.audio.inst_pos[NEW_LEVEL_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_stop_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                        }

                        mat.state = PAUSE;
                    }
                break;
                default:
                break;
            }
        }
        //**************************************//

        //**************************************//
        //game logic
        switch(mat.state)
        {
            case JEWEL:

                if (mistakes == 5){
                    //play easteregg
                    mistakes = 0;
                    al_stop_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                    al_play_sample_instance(mat.audio.sample_inst[TREASURE_AUDIO]);
                }


                if ( set_to_destroy_matched_jewels(&mat) ){
                    //plays destroy sound
                    if (al_get_sample_instance_playing(mat.audio.sample_inst[DESTROY_AUDIO]))
                        al_stop_sample_instance(mat.audio.sample_inst[DESTROY_AUDIO]);
                    al_play_sample_instance(mat.audio.sample_inst[DESTROY_AUDIO]);
                    
                    //atualiza arquivo com best score
                    if (mat.score > mat.best_score){
                        score_file = fopen("./resources/files/score_file.txt","w"); 
                        must_init(score_file, "arquivo de score");
                        fwrite(&mat.score, sizeof(int), 1, score_file);
                        fclose(score_file);
                        mat.best_score = mat.score;
                    }

                    set_falling(&mat); //sets jewels downward motion and creates new jewels
                    mat.state = DROP;
                }
                else{

                    if ( test_end_game(&mat) )
                        mat.state = END_GAME; 
                    else{

                        if ( mat.score >= mat.next_level_score  ){
                            if (al_get_sample_instance_playing(mat.audio.sample_inst[NEW_LEVEL_AUDIO]))
                                al_stop_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_play_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);

                            mat.level++;
                            mat.available_jewels = min((mat.available_jewels+1), MAX_AVAILABLE_JEWELS);
                            mat.state = NEW_LEVEL;
                            mat.next_level_score += (int)(1.5*mat.next_level_score);
                        }
                        else
                            mat.state = INPUT;
                    }

                }
                break;
            case INPUT:
                //process input
                if (mat.event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
                    if ( register_user_input(&mat.event, &mat) ){
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[SWAP_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[SWAP_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[SWAP_AUDIO]);
                        mat.state = SWAP;
                    }
                break;
            case SWAP:
                if(mat.event.type == ALLEGRO_EVENT_TIMER)
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
                            mat.state = DROP;
                        }
                        else if ( test_swap(&mat) ){
                            mistakes = 0;
                            mat.state = JEWEL;
                        }
                        else{
                            mistakes++;
                            //swap jewels
                            swap_jewels(mat.swap1, mat.swap2, mat.swap1->vel.x, mat.swap1->vel.y);
                            mat.state = DROP; //will update jewels unswapping;
                        }
                    }

                }
                break;
                case DROP:
                    if(mat.event.type == ALLEGRO_EVENT_TIMER){
                            moving = update_all_jewels(&mat);

                            if (!moving)
                                mat.state = JEWEL;
                    }
                break;
                case NEW_LEVEL:
                    if (mat.event.type == ALLEGRO_EVENT_TIMER)
                    {
                        if (framerate_divisor == NEW_LEVEL_SLOW_DOWN){ 
                            //in every frame_divisor amount of frames:
                            framerate_divisor = 0;
                            sort_jewels(&mat, SORTED_PER_FRAME);
                            new_lev_frames++;
                            if (new_lev_frames == NEW_LEVEL_TIMER){
                                new_lev_frames = 0;
                                mat.state  = WAIT;
                            }

                        }
                        else
                            framerate_divisor++;
                    }
                break;
                case WAIT:
                    if (mat.event.type == ALLEGRO_EVENT_TIMER)
                    {
                        if (count_frames == WAIT_FRAMES){ 
                            count_frames = 0;
                            mat.state  = JEWEL;
                        }
                        count_frames++;
                    }
                break;
            default:
                break;
        }
        //**************************************//


        //**************************************//
        //render game
        if(render && al_is_event_queue_empty(mat.queue))
        {
            render_game_frame(&mat);
            render = 0;
        }
        //**************************************//
        
    }

    //**************************************//
    deallocate_allegro_structures(&mat);
    //**************************************//
    free(mat.jewels);

    return 0;
}
