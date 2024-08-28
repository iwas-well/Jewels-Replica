CC = gcc
CFLAGS = -Wextra -o3 -Wall -std=c99
LDLIBS = -lallegro $(ALLEGRO_LIBS)
ALLEGRO_LIBS = `pkg-config allegro-5 allegro_ttf-5 allegro_font-5 \
			   allegro_image-5 allegro_primitives-5 allegro_audio-5 \
			   allegro_acodec-5 --libs --cflags`
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = jewels

all: $(TARGET)

$(TARGET): $(OBJ)

#game: jewels.o jewels_movement.o jewels_destruction.o libgame.o allegro_dependencies.o render_game.o
#	$(CC) $(CFLAGS) jewels.o jewels_movement.o jewels_destruction.o libgame.o allegro_dependencies.o  render_game.o $(ALLEGRO_LIBS) -o game
#jewels.o: jewels_destruction.h jewels_movement.h allegro_dependencies.h libgame.h structs.h jewels.c 
#	$(CC) $(CFLAGS) -c jewels.c
#jewels_movement.o: jewels_movement.h allegro_dependencies.h libgame.h structs.h jewels_movement.c
#	$(CC) $(CFLAGS) -c jewels_movement.c
#jewels_destruction.o: jewels_destruction.h allegro_dependencies.h libgame.h structs.h jewels_destruction.c
#	$(CC) $(CFLAGS) -c jewels_destruction.c
#render_game.o: allegro_dependencies.h libgame.h structs.h render_game.h render_game.c
#	$(CC) $(CFLAGS) -c render_game.c
#libgame.o: jewels_movement.h libgame.h structs.h allegro_dependencies.h libgame.c
#	$(CC) $(CFLAGS) -c libgame.c
#allegro_dependencies.o: allegro_dependencies.h libgame.h structs.h allegro_dependencies.c
#	$(CC) $(CFLAGS) -c allegro_dependencies.c

clean:
	-rm -f $(OBJ)

purge: clean
	-rm -f $(TARGET)
