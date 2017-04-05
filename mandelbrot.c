#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <SDL.h>
#include <SDL_image.h>

#define limit 6

static Uint32 hslToRGB(int angle);

static void setPixelRGB(SDL_Surface *image, int x, int y, int r, int g, int b) {
    Uint32 col = (0xff << 24) | (r << 16) | (g << 8) | b;

    Uint32 *pixels = (Uint32 *) image->pixels;
    pixels[ y * image->w + x] = col;
	//Uint16 col = (r << 11) + (g << 5) + b;

	//Uint16 *pixels = (Uint16 *) image->pixels;
	//pixels[y * image->w + x] = col;
}

static void setPixelHSL(SDL_Surface *image, int x, int y, int angle) {
	//printf("Xpos: %d, Ypos: %d\n", x, y);
	Uint32 *pixels = (Uint32 *) image->pixels;
	pixels[y * image->w + x] = hslToRGB(angle);
    //Uint32 col = (0xff << 24) | (0xff << 16) | (0x58 << 8) | 0xf0;
	//pixels[y * image->w + x] = col;
}

// MAGIC!! From: http://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion
static double hueToRGB(double p, double q, double t) {
	if (t < 0.0)
		t += 1.0;
	
	if (t > 1)
		t -= 1.0;

	if (t < (1.0/6.0))
		return p + (q - p) * 6.0 * t;

	if (t < 0.5)
		return q;

	if (t < (2.0/3.0))
		return p + (q - p) * ((2.0/3.0) - t) * 6.0;

	return p;
}

static Uint32 hslToRGB(int angle) {
	double r, g, b;
	double h = (angle % 360) / 360.0;
	//printf("Hue: %f\n", h);

	double q = 1.0; 
	double p = 0.0;
	r = hueToRGB(p, q, h + 1.0/3.0);
	g = hueToRGB(p, q, h);
	b = hueToRGB(p, q, h - 1.0/3.0);

	//printf("RGB: %f, %f, %f\n", r, g, b);

	int red = (int) round(r * 255.0);
	int green = (int) round(g * 255.0);
	int blue = (int) round(b * 255.0);

	return (0xff << 24) | (red << 16) | (green << 8) | blue;
}

static void mandelbrot(SDL_Surface *image) {
    //double r1, r2, c1, c2;
    //int count = 0;
	const int maxIter = 360;

	const int width = image->w;
	const int height = image->h;

	const double xmin = -0.9;
	const double xmax = -0.6;
	const double xstep = (xmax - xmin) / width;
	//printf("Xstep: %f\n", xstep);

	const double ymin = 0.0;
	const double ymax = 0.2;
	const double ystep = (ymax - ymin) / height;
	//printf("Ystep: %f\n", ystep);

	#pragma omp parallel for 
	for (int xpix = 0; xpix < width; xpix++) {
		for (int ypix = 0; ypix < height; ypix++) {
    		double r0, r1, r2, c0, c1, c2;
			int count;

            r0 = xmin + xpix * xstep;
            c0 = ymax - ypix * ystep;
			r1 = r0;
			c1 = c0;
            count = 0;
            while (count < maxIter && (r1 * r1) + (c1 * c1) < limit) {
                count++;
                r2 = r1 * r1 - c1 * c1 + r0;
                c2 = 2*r1*c1 + c0;

                r1 = r2;
                c1 = c2;
            }

            if (count >= maxIter) {
                setPixelRGB(image, xpix, ypix, 0, 0, 0);
            } else {
				setPixelHSL(image, xpix, ypix, count);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int max_threads = omp_get_max_threads();
    printf("Max threads: %d\n", max_threads);

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
		return 3;
    }

	printf("Height: %d, Width: %d\n", image->h, image->w);

    mandelbrot(image);

    //Uint32 pixelformat = image->format->format;
    //const char *formatName = SDL_GetPixelFormatName(pixelformat);
    //printf("Format: %s\n", formatName);
    //printf("BytesPerPixel: %d\n", image->format->BytesPerPixel);
    IMG_SavePNG(image, "out.png");
    
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
