#include "render_game.h"

void draw_jewel(game_struct* mat, jewel* j1) {
    ALLEGRO_BITMAP* sprite;

    float alpha= j1->alpha/100.0;

    if ((j1->type != EMPTY) && alpha){

        sprite=mat->image.sprite[ j1->type ];
        if (j1->power == DIAMOND)
            sprite=mat->image.sprite[ j1->type ];
        else if (j1->power == STAR)
            sprite=mat->image.star_sprite[ j1->type ];
        else if (j1->power == SQUARE)
            sprite=mat->image.square_sprite[ j1->type ];

        //al_draw_tinted_bitmap(sprite, , j1->current.x, j1->current.y, 0);
        al_draw_tinted_scaled_bitmap(sprite, al_map_rgba_f(1*alpha, 1*alpha, 1*alpha, alpha),
                0, 0, JEWEL_SPRITE_SIZE, JEWEL_SPRITE_SIZE, j1->current.x, j1->current.y, JEWEL_SIZE, JEWEL_SIZE, 0);
    }
}

void draw_ui(game_struct* mat)
{
    char score_text[50];

    //draw upper frame
    al_draw_bitmap(mat->image.screen[BG_IMAGE],0,0,0);

    sprintf(score_text, "%09d/%09d", mat->score, mat->next_level_score);
    al_draw_text(mat->font[SCORE_FONT], al_map_rgb(255,255,255), 80, 25, 0, score_text);

    sprintf(score_text, "BEST SCORE: %09d", mat->best_score);
    al_draw_text(mat->font[SCORE_FONT], al_map_rgb(255,255,255), SC_W-365, 25, 0, score_text);

    sprintf(score_text, "LEVEL: %02d", mat->level);
    al_draw_text(mat->font[SCORE_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-50, 25, 0, score_text);
}

int render_game_frame(game_struct* mat)
{
    //redraw_frame();
    if (mat->state == HELP_PAGE)
        al_draw_bitmap(mat->image.screen[HELP_IMAGE], 0, 0, 0);
    else
    {
        al_draw_bitmap(mat->image.screen[MAT_IMAGE],0,0, 0);

        //if one jewel is selected, draw a hint around it
        if (mat->selected) 
            al_draw_filled_rounded_rectangle(mat->swap1->proper.x, mat->swap1->proper.y,
                    mat->swap1->proper.x+JEWEL_SIZE, mat->swap1->proper.y+JEWEL_SIZE, 15, 15, al_map_rgba(35,35,60, 100));

        //draw jewel
        for (int i = 0; i< ROW_QT; i++)
            for (int j = 0; j < COL_QT; j++)
                draw_jewel(mat, &(mat->vanish[i][j]));

        //draw jewel
        for (int i = 0; i< ROW_QT; i++)
            for (int j = 0; j < COL_QT; j++)
                draw_jewel(mat, &(mat->jewels[i][j]));

        //first selected to swap is drawn above
        if (mat->swap2)
            draw_jewel(mat, mat->swap2);

        draw_ui(mat);

        if ((mat->state == NEW_LEVEL) || (mat->last_state == NEW_LEVEL)){
            //draw_new_level_screen(&game_images);
            al_draw_bitmap(mat->image.screen[TRANSPARENT_IMAGE], 0, 0, 0);
            al_draw_text(mat->font[GAME_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-90, SC_H-80, 0, "NEW LEVEL!");
        }

        if (mat->state == END_GAME){
            //draw end game screen
            al_draw_bitmap(mat->image.screen[TRANSPARENT_IMAGE], 0, 0, 0);
            al_draw_text(mat->font[GAME_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-90, SC_H-80, 0, "YOU LOST");
        }
        else if(mat->state == PAUSE){
            //draw pause game screen
            al_draw_bitmap(mat->image.screen[TRANSPARENT_IMAGE], 0, 0, 0);
            al_draw_text(mat->font[GAME_FONT], al_map_rgb(255,255,255), (int)(SC_W/2)-53, (int)(SC_H/2)-20, 0, "PAUSE");
        }
    }

    al_flip_display();

    return 0;
}
