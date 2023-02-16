#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "allegro_dependencies.h"
#include "libgame.h"
#include "jewels_destruction.h"
#include "jewels_movement.h"

//todo:
//  refactor code
//  rock
//  destruction animation

int main()
{
    game_struct mat;

    //could be in initialize_jewel_structure
    mat.pos.x = (SC_W - COL_QT*JEWEL_SIZE)/2;
    mat.pos.y = (SC_H - ROW_QT*JEWEL_SIZE)/2;
    mat.swap1 = NULL;
    mat.swap2 = NULL;

    if ( !initialize_jewel_structure(&mat) )
        fprintf(stdin,"erro ao alocar matriz de joias\n");

    initialize_allegro_dependencies(&mat);

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

    //could be in game_struct
    int state = JEWEL;
    int level = 1;
    int selected = 0;

    int moving;
    int count_frames = 0;
    int last_state;
    int new_lev_frames = 0; //amount of frames spent on new level state
    int next_level_score = FIRST_SCORE_GOAL;
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
                    if (state == HELP_PAGE)
                    {//quit help page
                        state = last_state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //sets audio rolling
                        al_play_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        al_set_sample_instance_position(mat.audio.sample_inst[BG_AUDIO], mat.audio.inst_pos[BG_AUDIO]);
                        if (last_state == NEW_LEVEL){
                            al_play_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_set_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO], mat.audio.inst_pos[NEW_LEVEL_AUDIO]);
                        }

                        last_state = HELP_PAGE;
                    }
                    else
                    {//enter help page
                        last_state = state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //stops easteregg song
                        al_stop_sample_instance(mat.audio.sample_inst[TREASURE_AUDIO]);

                        //saves audio position and stops it
                        mat.audio.inst_pos[BG_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[BG_AUDIO]);
                        al_stop_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        if (last_state == NEW_LEVEL){
                            mat.audio.inst_pos[NEW_LEVEL_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_stop_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                        }
                        state = HELP_PAGE;
                    }
                break;
                case ALLEGRO_KEY_ESCAPE: //fallthrough
                case ALLEGRO_KEY_SPACE:
                    if (state == PAUSE)
                    {//unpause game
                        state = last_state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //sets audio rolling
                        al_play_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        al_set_sample_instance_position(mat.audio.sample_inst[BG_AUDIO], mat.audio.inst_pos[BG_AUDIO]);
                        if (last_state == NEW_LEVEL){
                            al_play_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_set_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO], mat.audio.inst_pos[NEW_LEVEL_AUDIO]);
                        }
                        last_state = PAUSE;
                    }
                    else if ((state != HELP_PAGE) && (state != END_GAME))
                    {//pause game
                        last_state = state;

                        //plays pause audio
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[PAUSE_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[PAUSE_AUDIO]);

                        //stops easteregg song
                        al_stop_sample_instance(mat.audio.sample_inst[TREASURE_AUDIO]);
                         
                        //saves audio position and stops it
                        mat.audio.inst_pos[BG_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[BG_AUDIO]);
                        al_stop_sample_instance(mat.audio.sample_inst[BG_AUDIO]);
                        if (last_state == NEW_LEVEL){
                            mat.audio.inst_pos[NEW_LEVEL_AUDIO] = al_get_sample_instance_position(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_stop_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                        }

                        state = PAUSE;
                    }
                break;
                default:
                break;
            }
        }
        //**************************************//

        //**************************************//
        //game logic
        switch(state)
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
                    if (mat.score > best_score){
                        score_file = fopen("./resources/files/score_file.txt","w"); 
                        must_init(score_file, "arquivo de score");
                        fwrite(&mat.score, sizeof(int), 1, score_file);
                        fclose(score_file);
                        best_score = mat.score;
                    }

                    set_falling(&mat); //sets jewels downward motion and creates new jewels
                    state = DROP;
                }
                else{

                    if ( test_end_game(&mat) )
                        state = END_GAME; 
                    else{

                        if ( mat.score >= next_level_score  ){
                            if (al_get_sample_instance_playing(mat.audio.sample_inst[NEW_LEVEL_AUDIO]))
                                al_stop_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);
                            al_play_sample_instance(mat.audio.sample_inst[NEW_LEVEL_AUDIO]);

                            level++;
                            mat.available_jewels = min((mat.available_jewels+1), MAX_AVAILABLE_JEWELS);
                            state = NEW_LEVEL;
                            next_level_score += (int)(1.5*next_level_score);
                        }
                        else
                            state = INPUT;
                    }

                }
                break;
            case INPUT:
                //process input
                if (mat.event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
                    if ( register_user_input(&mat.event, &mat, &selected) ){
                        if (al_get_sample_instance_playing(mat.audio.sample_inst[SWAP_AUDIO]))
                            al_stop_sample_instance(mat.audio.sample_inst[SWAP_AUDIO]);
                        al_play_sample_instance(mat.audio.sample_inst[SWAP_AUDIO]);
                        state = SWAP;
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
                    if(mat.event.type == ALLEGRO_EVENT_TIMER){
                            moving = update_all_jewels(&mat);

                            if (!moving)
                                state = JEWEL;
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
                                state  = WAIT;
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
                            state  = JEWEL;
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
            //redraw_frame();
            if (state == HELP_PAGE)
            {
                al_draw_bitmap(mat.image.screen[HELP_IMAGE], 0, 0, 0);
                al_flip_display();
            }
            else{

                al_draw_bitmap(mat.image.screen[MAT_IMAGE],0,0, 0);


                //if one jewel is selected, draw a hint around it
                if (selected) 
                    al_draw_filled_rounded_rectangle(mat.swap1->proper.x, mat.swap1->proper.y,
                            mat.swap1->proper.x+JEWEL_SIZE, mat.swap1->proper.y+JEWEL_SIZE, 15, 15, al_map_rgba(35,35,60, 100));

                //draw jewel
                for (int i = 0; i< ROW_QT; i++)
                    for (int j = 0; j < COL_QT; j++)
                        if (mat.jewels[i][j].type != EMPTY){
                            if (mat.jewels[i][j].power == NONE)
                                al_draw_bitmap(mat.image.sprite[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                            else if (mat.jewels[i][j].power == STAR){
                                al_draw_bitmap(mat.image.star_sprite[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                            }
                            else if (mat.jewels[i][j].power == SQUARE)
                                al_draw_bitmap(mat.image.square_sprite[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                            else if (mat.jewels[i][j].power == DIAMOND)
                                al_draw_bitmap(mat.image.sprite[ mat.jewels[i][j].type ],
                                        mat.jewels[i][j].current.x, mat.jewels[i][j].current.y, 0);
                        }

                //draw_jewel()
                //first selected to swap is drawn above
                if (mat.swap2)
                    if (mat.swap2->type != EMPTY ){
                        if (mat.swap2->power == NONE)
                            al_draw_bitmap(mat.image.sprite[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                        else if (mat.swap2->power == DIAMOND)
                            al_draw_bitmap(mat.image.sprite[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                        else if (mat.swap2->power == STAR)
                            al_draw_bitmap(mat.image.star_sprite[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                        else if (mat.swap2->power == SQUARE)
                            al_draw_bitmap(mat.image.square_sprite[ mat.swap2->type ],
                                    mat.swap2->current.x, mat.swap2->current.y, 0);
                    }

                //draw upper frame
                al_draw_bitmap(mat.image.screen[BG_IMAGE],0,0,0);

                //**************************************************************//
                //draw UI
                char score_text[19];
                sprintf(score_text, "%09d/%09d", mat.score, next_level_score);
                al_draw_text(mat.font[SCORE_FONT], al_map_rgb(255,255,255), 80, 25, 0, score_text);

                sprintf(score_text, "BEST SCORE: %09d", best_score);
                al_draw_text(mat.font[SCORE_FONT], al_map_rgb(255,255,255), SC_W-365, 25, 0, score_text);

                sprintf(score_text, "LEVEL: %02d", level);
                al_draw_text(mat.font[SCORE_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-50, 25, 0, score_text);
                //**************************************************************//


                if ((state == NEW_LEVEL) || (last_state == NEW_LEVEL)){
                    //draw_new_level_screen(&game_images);
                    al_draw_bitmap(mat.image.screen[TRANSPARENT_IMAGE], 0, 0, 0);
                    al_draw_text(mat.font[GAME_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-90, SC_H-80, 0, "NEW LEVEL!");
                }

                if (state == END_GAME){
                    //draw end game screen
                    al_draw_bitmap(mat.image.screen[TRANSPARENT_IMAGE], 0, 0, 0);
                    al_draw_text(mat.font[GAME_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-90, SC_H-80, 0, "YOU LOST");
                }
                else if(state == PAUSE){
                    al_draw_bitmap(mat.image.screen[TRANSPARENT_IMAGE], 0, 0, 0);
                    al_draw_text(mat.font[GAME_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-53, (int)(SC_H/2)-20, 0, "PAUSE");
                }


                
                al_flip_display();
            }
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
