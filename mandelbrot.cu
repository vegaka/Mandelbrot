#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cuComplex.h>

#define LIMIT 4
#define IMG_SIZE 4096
#define MAX_ITERATIONS 250

__device__ static Uint32 hslToRGB(int angle);

__device__ static void setPixelRGB(Uint32 *pixels, const int x, const int y,
	       		       	   const int r, const int g, const int b) {
    pixels[y * IMG_SIZE + x] = (0xff << 24) | (r << 16) | (g << 8) | b;
}

__device__ static void setPixelHSL(Uint32 *pixels, const int x, const int y, const int angle) {
	pixels[y * IMG_SIZE + x] = hslToRGB(angle);
}

// MAGIC!! From: http://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion
__device__ static double hueToRGB(const double p, const double q, double t) {
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

__device__ static Uint32 hslToRGB(const int angle) {
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

__global__ static void mandelbrot(Uint32 *image, const double xmin, const double xmax,
                                  const double ymin, const double ymax) {
    const double xstep = (xmax - xmin) / IMG_SIZE;
    const double ystep = (ymax - ymin) / IMG_SIZE;

    const int xpix = blockIdx.x * blockDim.x + threadIdx.x;
    const int ypix = blockIdx.y;

    cuDoubleComplex c = make_cuDoubleComplex(xmin + xpix * xstep, ymin + ypix * ystep);
    cuDoubleComplex z = make_cuDoubleComplex(0.0, 0.0);

    int count = 0;
    
    while (count < MAX_ITERATIONS && cuCabs(z) < LIMIT) {
        count++;

        z = cuCmul(z, z);
        z = cuCadd(z, c);
    }

    if (count >= MAX_ITERATIONS) {
        setPixelRGB(image, xpix, ypix, 0, 0, 0);
    } else {
        setPixelHSL(image, xpix, ypix, count);
    }
}

int main(int argc, char *argv[]) {
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
    if (image->h != IMG_SIZE && image->w != IMG_SIZE) {
	    fprintf(stderr, "Unexpected image dimensions: (%d, %d). Expected (%d, %d).\n", image->w, image->h, IMG_SIZE, IMG_SIZE);
	    return 4;
    }

    Uint32 *pixels;
    cudaMallocManaged(&pixels, IMG_SIZE * IMG_SIZE * sizeof(Uint32));
    delete [] (Uint32 *)image->pixels;
    image->pixels = pixels;

    const double xmin = -0.7463;
    const double xmax = -0.7473;
    const double ymin = 0.1102;
    const double ymax = 0.1112;
    /*const double xmin = -2;
    const double xmax = 1;
    const double ymin = -1.5;
    const double ymax = 1.5;*/

    int threadsPerBlock = 256;
    int numBlocks = IMG_SIZE / threadsPerBlock;
    dim3 grid(numBlocks, IMG_SIZE);
    mandelbrot<<<grid, threadsPerBlock>>>(pixels, xmin, xmax, ymin, ymax);
    cudaDeviceSynchronize();

//    (Uint32 *) (image->pixels) = pixels;
    //Uint32 *imgPixels = (Uint32 *) image->pixels;
    //for (int i = 0; i < IMG_SIZE * IMG_SIZE; i++) {
//        imgPixels[i] = pixels[i];
 //   }

    /*
    int size = 4;
    int tPB = 2;
    int numBlocks = size / tPB;
    dim3 grid(numBlocks, size);
    test<<<grid, tPB>>>();
    cudaDeviceSynchronize();
    */


    //mandelbrot(image);

    //Uint32 pixelformat = image->format->format;
    //const char *formatName = SDL_GetPixelFormatName(pixelformat);
    //printf("Format: %s\n", formatName);
    //printf("BytesPerPixel: %d\n", image->format->BytesPerPixel);
    IMG_SavePNG(image, "out.png");
    
    cudaFree(pixels);
    //SDL_FreeSurface(image); Causes segfault
    IMG_Quit();
    SDL_Quit();

    return 0;
}
