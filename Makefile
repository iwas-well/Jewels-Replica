CC = gcc
CFLAGS = -g -Wall -std=c99
ALLEGRO_LIBS = `pkg-config allegro-5 allegro_ttf-5 allegro_font-5 \
			   allegro_image-5 allegro_primitives-5 allegro_audio-5 \
			   allegro_acodec-5 --libs --cflags`

all: game


game: jewels.o jewels_movement.o jewels_destruction.o libgame.o allegro_dependencies.o render_game.o
	$(CC) jewels.o jewels_movement.o jewels_destruction.o libgame.o allegro_dependencies.o  render_game.o $(ALLEGRO_LIBS) -o game

jewels.o: jewels_destruction.h jewels_movement.h allegro_dependencies.h libgame.h structs.h jewels.c 
	$(CC) -c jewels.c

jewels_movement.o: jewels_movement.h allegro_dependencies.h libgame.h structs.h jewels_movement.c
	$(CC) -c jewels_movement.c

jewels_destruction.o: jewels_destruction.h allegro_dependencies.h libgame.h structs.h jewels_destruction.c
	$(CC) -c jewels_destruction.c

render_game.o: allegro_dependencies.h libgame.h structs.h render_game.h render_game.c
	$(CC) -c render_game.c

libgame.o: jewels_movement.h libgame.h structs.h allegro_dependencies.h libgame.c
	$(CC) -c libgame.c

allegro_dependencies.o: allegro_dependencies.h libgame.h structs.h allegro_dependencies.c
	$(CC) -c allegro_dependencies.c

clean:
	-rm -f *.o

purge: clean
	-rm -f game
