#include <stdlib.h>
#include <stdio.h>
//#include <omp.h>
#include <SDL.h>
#include <SDL_image.h>

#define limit 4
#define scale 0.01 

static void setPixel(SDL_Surface *image, int x, int y, int r, int g, int b) {
    Uint32 col = (0xffff << 24) + (r << 16) + (g << 8) + b;

    Uint32 *pixels = (Uint32 *) image->pixels;
    pixels[ y * image->w + x] = col;
}

static void mandelbrot(SDL_Surface *image) {
    double xpos;
    double ypos;
    double r1, r2, c1, c2;
    int count = 0;

    for (int i = -(image->w / 2); i < image->w / 2; i++) {
        for (int j = -(image->h / 2); j < image->h / 2; j++) {
            xpos = i * scale;
            ypos = j * scale;
           
            r1 = 0;
            c1 = 0;
            count = 0;
            while (count <= 255 && (r1 * r1) + (c1 * c1) < limit) {
                count++;
                r2 = r1 * r1 - c1 * c1 + xpos;
                c2 = 2*r1*c1 + ypos;

                r1 = r2;
                c1 = c2;
            }

            if (count > 255) {
                setPixel(image, i + image->w / 2, j + image->h / 2, 0, 0, 0);
                printf("Didn't converge.\n");
            } else {
                setPixel(image, i + image->w / 2, j + image->h / 2, 0xffff, 0xffff, 0xffff);
            }
        }
    }
    
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

    SDL_Surface *image = IMG_Load("out.png");
    if (!image) {
        SDL_Log("IMG_Load error: %s\n", IMG_GetError());
    }

    //for (int i = 0; i < image->w; i++) {
    //    for (int j = 0; j < image->h; j++) {
    //        setPixel(image, i, j, 0b11111, 0, 0);
    //    }
    //}
    
    mandelbrot(image);

    Uint32 pixelformat = image->format->format;
    const char *formatName = SDL_GetPixelFormatName(pixelformat);
    printf("Format: %s\n", formatName);
    printf("BytesPerPixel: %d\n", image->format->BytesPerPixel);
    IMG_SavePNG(image, "out.png");
    
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
