#include <stdio.h>
#include <stdlib.h>

#include "render_game.h"
#include "structs.h"
#include "allegro_dependencies.h"
#include "libgame.h"
#include "jewels_destruction.h"
#include "jewels_movement.h"

#define SCORE_FILE_PATH "./resources/files/score_file.txt"

void change_state(game_struct *mat, int new_state) {
    //enters help page
    mat->last_state = mat->state;
    mat->state = new_state;
}

void set_timer(game_struct *mat, float sec){
    al_set_timer_speed(mat->timer2, sec);
    al_start_timer(mat->timer2);
}

void set_frame_rate(game_struct *mat, float sec){
    al_set_timer_speed(mat->timer, sec);
    al_start_timer(mat->timer);
}

void reset_frame_rate(game_struct *mat){
    set_frame_rate(mat, 1.0/FPS);
}

void stop_timer(game_struct *mat) {
    al_stop_timer(mat->timer2);
}

int jewel_state_logic(game_struct *mat) {
    //creates powerups, destroy jewels and set them in motion
    if ( set_to_destroy_matched_jewels(mat) ) {
        //plays destroy sound
        if (al_get_sample_instance_playing(mat->audio.sample_inst[DESTROY_AUDIO]))
            al_stop_sample_instance(mat->audio.sample_inst[DESTROY_AUDIO]);
        al_play_sample_instance(mat->audio.sample_inst[DESTROY_AUDIO]);

        //atualiza arquivo com best score
        if (mat->score > mat->best_score){
            FILE *score_file;
            if ( !(score_file = fopen("./resources/files/score_file.txt","w")) ){
                fprintf(stdin,"erro ao abrir arquivo de score\n");
                return 1;
            }
            fwrite(&(mat->score), sizeof(int), 1, score_file);
            fclose(score_file);
            mat->best_score = mat->score;
        }
        set_falling(mat); //sets jewels downward motion and creates new jewels
        change_state(mat, UPDATE);
    }
    else {
        if ( test_end_game(mat) == 1 )
            change_state(mat, END_GAME);
        else{
            if ( mat->score >= mat->next_level_score  ){
                //play new level song
                if (al_get_sample_instance_playing(mat->audio.sample_inst[NEW_LEVEL_AUDIO]))
                    al_stop_sample_instance(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
                al_play_sample_instance(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);

                //increase level and set new next level goal
                mat->level++;
                //increase number of available jewels
                mat->available_jewels = min((mat->available_jewels+1), MAX_AVAILABLE_JEWELS);
                mat->next_level_score += (int)(1.5*mat->next_level_score);

                set_timer(mat, NEW_LEVEL_TIMER);
                change_state(mat, NEW_LEVEL);
            }
            else
                change_state(mat, INPUT);
        }
    }
    return 0;
}

//restore music position and sets it to play
void continue_music(game_struct *mat) {
    al_play_sample_instance(mat->audio.sample_inst[BG_AUDIO]);
    al_set_sample_instance_position(mat->audio.sample_inst[BG_AUDIO], mat->audio.inst_pos[BG_AUDIO]);
    if (mat->last_state == NEW_LEVEL) {
        al_play_sample_instance(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
        al_set_sample_instance_position(mat->audio.sample_inst[NEW_LEVEL_AUDIO], mat->audio.inst_pos[NEW_LEVEL_AUDIO]);
    }
}

//saves music position and stops it
void pause_music(game_struct *mat) {
    mat->audio.inst_pos[BG_AUDIO] = al_get_sample_instance_position(mat->audio.sample_inst[BG_AUDIO]);
    al_stop_sample_instance(mat->audio.sample_inst[BG_AUDIO]);
    if (mat->state == NEW_LEVEL) {
        mat->audio.inst_pos[NEW_LEVEL_AUDIO] = al_get_sample_instance_position(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
        al_stop_sample_instance(mat->audio.sample_inst[NEW_LEVEL_AUDIO]);
    }
}

int load_best_score(char* file_path){
    FILE* fd;
    int best_score = 0;
    if ( (fd = fopen(file_path,"r")) ) {
        fread(&best_score, sizeof(int), 1, fd);
        fclose(fd);
    }
    return best_score;
}

void toggle_pause_state(game_struct *mat, int state) {
    al_stop_sample_instance(mat->audio.sample_inst[PAUSE_AUDIO]);
    al_play_sample_instance(mat->audio.sample_inst[PAUSE_AUDIO]);
    if (mat->state == state) {
        continue_music(mat);
        change_state(mat, mat->last_state);
    }
    else{
        //stops easter egg song
        al_stop_sample_instance(mat->audio.sample_inst[TREASURE_AUDIO]);
        pause_music(mat);
        change_state(mat, state);
    }
}

void process_keyboard_input(game_struct *mat) {
    switch (mat->event.keyboard.keycode) {
        case ALLEGRO_KEY_F1: //fallthrough
        case ALLEGRO_KEY_H:
            toggle_pause_state(mat, HELP_PAGE);
            break;
        case ALLEGRO_KEY_ESCAPE: //fallthrough
        case ALLEGRO_KEY_SPACE:
            if ((mat->state != HELP_PAGE) && (mat->state != END_GAME))
                toggle_pause_state(mat, PAUSE);
            break;
        default:
            break;
    }
}

int input_state_logic(game_struct *mat) {
    if (mat->event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        register_user_selection(&(mat->event), mat);
        if (mat->selected == 2) { //two jewels were selected
            mat->selected = 0;
            set_swap_direction(mat);
            if (mat->swap_direction != STALL)
                change_state(mat, SWAP);
        }
    }

    return 0;
}

int swap_state_logic(game_struct *mat) {
    if (mat->swap_direction == STALL) {
        change_state(mat, JEWEL);
        return 0;
    }

    if (mat->last_state == INPUT) { //start swapping
                                    //play swap sound
        al_stop_sample_instance(mat->audio.sample_inst[SWAP_AUDIO]);
        al_play_sample_instance(mat->audio.sample_inst[SWAP_AUDIO]);

        swap_jewels(mat->swap1, mat->swap2, mat->swap_direction, VELOCITY_PER_FRAME);

        change_state(mat, UPDATE);
        return 0;
    }

    //keep in mind swap1 must be initialized
    if ((mat->swap1->power == DIAMOND) || (mat->swap2->power == DIAMOND)) {
        mat->mistake_counter = 0;
        //no need to test swap
        jewel *diamond = mat->swap1;
        j_type destroy_type = mat->swap2->type;
        if (mat->swap2->power == DIAMOND) {
            diamond = mat->swap2;
            destroy_type = mat->swap1->type;
        }
        vec2 rowcol = get_rowcol(diamond->proper.x, diamond->proper.y, mat);
        destroy_jewel_on_slot(mat, rowcol.row, rowcol.col, destroy_type);
        set_falling(mat); //set jewels downward motion and creates new jewels
        mat->swap_direction=STALL;
        change_state(mat, UPDATE);
    }
    else if ( test_swap(mat) ) {
        mat->mistake_counter = 0;
        change_state(mat, JEWEL);
    }
    else { //unswap jewels
        if (++(mat->mistake_counter) == 5) {
            mat->mistake_counter = 0;
            //play easter egg song
            al_stop_sample_instance(mat->audio.sample_inst[BG_AUDIO]);
            al_play_sample_instance(mat->audio.sample_inst[TREASURE_AUDIO]);
        }
        swap_jewels(mat->swap1, mat->swap2, mat->swap_direction, VELOCITY_PER_FRAME);
        mat->swap_direction=STALL;
        change_state(mat, UPDATE);
    }

    return 0; 
}

int handle_game_state(game_struct *mat) {
    switch ( mat->state ) { //game state machine
        case JEWEL:
            if (jewel_state_logic(mat) != 0) return 1;
            break;
        case INPUT: //user input state
            if (input_state_logic(mat) != 0) return 1;
            break;
        case SWAP: //update swapped jewels position, unswaps them if no match was formed
            if(mat->event.type == ALLEGRO_EVENT_TIMER) //make it frame rate consistent
                if (swap_state_logic(mat) != 0) return 1;
            break;
        case UPDATE: //update all jewels positions, then return to last state
            if(mat->event.type == ALLEGRO_EVENT_TIMER) {
                int still_moving = update_all_jewels(mat);
                if (!still_moving) {
                    if ((mat->last_state == PAUSE) || (mat->last_state == HELP_PAGE))
                        change_state(mat, JEWEL);
                    else
                        change_state(mat, mat->last_state);
                }
            }
            break;
        case NEW_LEVEL:
            if (mat->event.type == ALLEGRO_EVENT_TIMER) {
                sort_jewels(mat, SORTED_PER_FRAME);
                if (mat->event.timer.source == mat->timer2) {
                    set_timer(mat, 0.5);
                    change_state(mat, WAIT);
                }
            }
            break;
        case WAIT: //await for timer event, then change state to JEWEL
            if ((mat->event.type == ALLEGRO_EVENT_TIMER) &&
                (mat->event.timer.source == mat->timer2))
            {
                stop_timer(mat);
                change_state(mat, JEWEL);
            }
            break;
        default:
            break;
    }

    return 0;
}

int main()
{
    game_struct mat;

    if ( !initialize_jewel_structure(&mat) ){
        fprintf(stdin,"erro ao alocar matriz de joias\n");
        return 1;
    }
    initialize_allegro_dependencies(&mat);

    mat.best_score = load_best_score(SCORE_FILE_PATH);

    while (1) { //game loop

        //wait for event queue to recieve an event.
        //first event of the queue is then moved to mat.event
        al_wait_for_event(mat.queue, &mat.event);

        //close game window
        if(mat.event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;

        if (mat.event.type == ALLEGRO_EVENT_KEY_DOWN)
            process_keyboard_input(&mat); //treat state independent keyboard input (pause and help)

        if (handle_game_state(&mat) != 0)
            break;

        if( al_is_event_queue_empty(mat.queue) )
            render_game_frame(&mat);

    }

    deallocate_allegro_structures(&mat);
    free(mat.jewels);

    return 0;
}
