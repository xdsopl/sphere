
CFLAGS = -std=c99 -Wall -W -O3 $(shell sdl-config --cflags)
LDLIBS = $(shell sdl-config --libs) -lm -lGL

sphere: sphere.c

clean:
	rm -f sphere

