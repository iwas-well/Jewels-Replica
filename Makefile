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

clean:
	-rm -f $(wildcard src/*.o)

purge: clean
	-rm -f $(TARGET)
