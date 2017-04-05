CC=cc
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)
CFLAGS+=-std=c99 -g -Wall -fopenmp -lm

mandelbrot: mandelbrot.c
	$(CC) mandelbrot.c $(SDL_CFLAGS) $(SDL_LDFLAGS) -lSDL2_image $(CFLAGS) -o mandelbrot


.PHONY: clean
clean:
	-rm -f mandelbrot
