#include <stdio.h>
#include <stdlib.h>

#include "src/allegro_dependencies.h"
#include "src/structs.h"
#include "src/libgame.h"
#include "src/game_states.h"
#include "src/render_game.h"

static void process_keyboard_input(game_struct *mat) {
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

static int load_best_score(char* file_path){
    FILE* fd;
    int best_score = 0;
    if ( (fd = fopen(file_path,"r")) ) {
        fread(&best_score, sizeof(int), 1, fd);
        fclose(fd);
    }
    return best_score;
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
    free(mat.destroyed);

    return 0;
}
