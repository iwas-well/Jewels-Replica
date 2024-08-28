#include <stdio.h>

#include "game_states.h"
#include "structs.h"
#include "libgame.h"
#include "jewels_destruction.h"
#include "jewels_movement.h"
#include "render_game.h"

int handle_game_state(game_struct *mat) {
    switch ( mat->state ) { //game state machine
        case JEWEL:
            if (jewel_state_logic(mat) != 0) return 1;
            break;
        case INPUT: //user input registering state
            if (input_state_logic(mat) != 0) return 1;
            break;
        case SWAP: //set swapped jewels movement, unswaps them if no match was formed
            if(mat->event.type == ALLEGRO_EVENT_TIMER)
                if (swap_state_logic(mat) != 0) return 1;
            break;
        case UPDATE: //update all jewels positions, then return to last state
            if(mat->event.type == ALLEGRO_EVENT_TIMER)
                if (update_sate_logic(mat) != 0) return 1;
            break;
        case NEW_LEVEL:
            if (mat->event.type == ALLEGRO_EVENT_TIMER)
                if (new_level_state_logic(mat) != 0) return 1;
            break;
        case WAIT: //await for timer event, then change state to JEWEL
                if (wait_state_logic(mat) != 0) return 1; 
            break;
        default:
            break;
    }
    return 0;
}

int jewel_state_logic(game_struct *mat) {
    //creates powerups, destroy jewels and set them in motion
    if ( set_to_destroy_matched_jewels(mat) ) {
        //plays destroy sound
        if (al_get_sample_instance_playing(mat->audio.sample_inst[DESTROY_AUDIO]))
            al_stop_sample_instance(mat->audio.sample_inst[DESTROY_AUDIO]);
        al_play_sample_instance(mat->audio.sample_inst[DESTROY_AUDIO]);

        //atualiza arquivo com best score
        if (mat->score > mat->best_score)
            if (save_best_score(mat) != 0)
                return 1;

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

int update_sate_logic(game_struct *mat) {
    int still_moving = update_all_jewels(mat);
    if (!still_moving) {
        if ((mat->last_state == PAUSE) || (mat->last_state == HELP_PAGE))
            change_state(mat, JEWEL);
        else
            change_state(mat, mat->last_state);
    }
    return 0;
}

int new_level_state_logic(game_struct *mat) {
    sort_jewels(mat, SORTED_PER_FRAME);
    if (mat->event.timer.source == mat->timer) {
        set_timer(mat, 0.5);
        change_state(mat, WAIT);
    }
    return 0;
}

int wait_state_logic(game_struct *mat) {
    if ((mat->event.type == ALLEGRO_EVENT_TIMER) && 
            (mat->event.timer.source == mat->timer))
    {
        stop_timer(mat);
        change_state(mat, JEWEL);
    }
    return 0;
}
