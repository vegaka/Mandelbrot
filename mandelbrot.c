#include <stdlib.h>
#include <stdio.h>
//#include <omp.h>
#include <SDL.h>
#include <SDL_image.h>

#define limit 4
#define size 2

static void setPixel(SDL_Surface *image, int x, int y, int r, int g, int b) {
    Uint16 col = r << 10 + g << 5 + b;

    Uint16 *pixels = (Uint16 *) image->pixels;
    pixels[ y * image->w + x] = col;
}

static void mandelbrot(SDL_Surface *image) {
    int x0 = image->w / 2;
    int y0 = image->h / 2;
    double real;
    double comp;
    int count = 0;


    
}

int main(int argc, char *argv[]) {
    //int max_threads = omp_get_max_threads();
    //printf("Max threads: %d\n", max_threads);

    if (SDL_Init(0) != 0) {
        SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Log("Unable to initialize SDL_Image: %s\n", IMG_GetError());
        return 2;
    }

    SDL_Surface *image = IMG_Load("image.png");
    if (!image) {
        SDL_Log("IMG_Load error: %s\n", IMG_GetError());
    }

    for (int i = 0; i < image->w; i++) {
        for (int j = 0; j < image->h; j++) {
            setPixel(image, i, j, 0b11111, 0, 0);
        }
    }

    Uint32 pixelformat = image->format->format;
    char *formatName = SDL_GetPixelFormatName(pixelformat);
    printf("Format: %s\n", formatName);
    printf("BytesPerPixel: %d\n", image->format->BytesPerPixel);
    IMG_SavePNG(image, "out.png");
    
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
