#gcc jewels.c -o teste $(pkg-config allegro-5 allegro_font-5 allegro_image-5 allegro_primitives-5 --libs --cflags); ./teste
CC = gcc
CFLAGS = -g -Wall -std=c99
ALLEGRO_LIBS = `pkg-config allegro-5 allegro_ttf-5 allegro_font-5 \
			   allegro_image-5 allegro_primitives-5 allegro_audio-5 \
			   allegro_acodec-5 --libs --cflags`

all: game

game: jewels.o
	$(CC) -o game $(CFLAGS) jewels.o $(ALLEGRO_LIBS)

jewels.o: jewels.c
	$(CC) -c -g jewels.c

clean:
	-rm -f *.o

purge: clean
	-rm -f game
