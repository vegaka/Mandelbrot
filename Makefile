CC=cc
NVCC=nvcc
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)
CFLAGS+=-std=c99 -g -Wall -fopenmp -lm#-Xpreprocessor -fopenmp #-lomp
NVCCFLAGS+= -g -lm

mandelbrot: mandelbrot.c
	$(CC) mandelbrot.c $(CFLAGS) $(SDL_CFLAGS) $(SDL_LDFLAGS) -lSDL2_image -o mandelbrot

cuda: mandelbrot.cu
	$(NVCC) mandelbrot.cu $(NVCCFLAGS) $(SDL_CFLAGS) $(SDL_LDFLAGS) -lSDL2_image -o mandelbrot_cu

.PHONY: clean
clean:
	-rm -f mandelbrot
	-rm -f mandelbrot_cu
