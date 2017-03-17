#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <omp.h>
#include <SDL.h>
#include <SDL_image.h>

#define limit 6

static void setPixel(SDL_Surface *image, int x, int y, int r, int g, int b) {
    Uint32 col = (0xff << 24) | (r << 16) | (g << 8) | b;

    Uint32 *pixels = (Uint32 *) image->pixels;
    pixels[ y * image->w + x] = col;
	//Uint16 col = (r << 11) + (g << 5) + b;

	//Uint16 *pixels = (Uint16 *) image->pixels;
	//pixels[y * image->w + x] = col;
}

// MAGIC!! From: http://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion
static double hueToRGB(double p, double q, double t) {
	if (t < 0)
		t += 1;
	
	if (t > 1)
		t -= 1;

	if (t < (1/6))
		return p + (q - p) * 6 * t;

	if (t < 0.5)
		return q;

	if (t < (2/3))
		return p + (q - p) * ((2/3) - t) * 6;

	return p;
}

static Uint32 hslToRGB(int count) {
	double r, g, b;
	double h = count / 360;

	double q = 1.0; 
	double p = 0.0;
	r = hueToRGB(p, q, h + 1/3);
	g = hueToRGB(p, q, h);
	b = hueToRGB(p, q, h - 1/3);

	int red = (int) round(r * 255);
	int green = (int) round(g * 255);
	int blue = (int) round(b * 255);

	//printf("RGB: %f, %f, %f\n", r, g, b);

	return (0xff << 24) | (red << 16) | (green << 8) | blue;
}

static void mandelbrot(SDL_Surface *image) {
    double r1, r2, c1, c2;
    int count = 0;
	int maxIter = 50;

	int width = image->w;
	int height = image->h;

	double xmin = -2.0;
	double xmax = 1.5;
	double xstep = (xmax - xmin) / width;
	printf("Xstep: %f\n", xstep);

	double ymin = -2.0;
	double ymax = 1.5;
	double ystep = (ymax - ymin) / height;
	printf("Ystep: %f\n", ystep);

	int xpix = 0;
	int ypix = height;

	//int colStep = (1 << 24) / (256);

    for (double x = xmin; x < xmax; x += xstep) {
        for (double y = ymin; y < ymax; y += ystep) { 
            r1 = 0;
            c1 = 0;
            count = 0;
            while (count < maxIter && (r1 * r1) + (c1 * c1) < limit) {
                count++;
                r2 = r1 * r1 - c1 * c1 + x;
                c2 = 2*r1*c1 + y;

                r1 = r2;
                c1 = c2;
            }

            if (count >= maxIter) {
                setPixel(image, xpix, ypix, 0, 0, 0);
                printf("Didn't converge.\n");
            } else {
				//int col = count * colStep;
                //setPixel(image, xpix, ypix, col & 0xff0000, col & 0x00ff00, col & 0x0000ff);
				Uint32 col = hslToRGB(count);
//				printf("Count: %d\n", count);
                setPixel(image, xpix, ypix, col & 0x00ff0000, col & 0x0000ff00, col & 0x000000ff);
            }

			ypix--;
        }
		ypix = height;
		xpix++;
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
