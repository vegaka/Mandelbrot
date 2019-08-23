#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>

#define DEFAULT_SIZE 1024
#define limit 6

static Uint32 hslToRGB(int angle);

struct ImageMetadata {
    int size;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    char * path;
};

static void setPixelRGB(SDL_Surface *image, int x, int y, int r, int g, int b) {
    Uint32 col = (0xff << 24) | (r << 16) | (g << 8) | b;

    Uint32 *pixels = (Uint32 *) image->pixels;
    pixels[ y * image->w + x] = col;
}

static void setPixelHSL(SDL_Surface *image, int x, int y, int angle) {
	Uint32 *pixels = (Uint32 *) image->pixels;
	pixels[y * image->w + x] = hslToRGB(angle);
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

	double q = 1.0; 
	double p = 0.0;
	r = hueToRGB(p, q, h + 1.0/3.0);
	g = hueToRGB(p, q, h);
	b = hueToRGB(p, q, h - 1.0/3.0);

	int red = (int) round(r * 255.0);
	int green = (int) round(g * 255.0);
	int blue = (int) round(b * 255.0);

	return (0xff << 24) | (red << 16) | (green << 8) | blue;
}

static void mandelbrot(SDL_Surface *image, struct ImageMetadata data) {
	const int maxIter = 720;

	const int width = image->w;
	const int height = image->h;

	const double xmin = data.xmin;
	const double xmax = data.xmax;
	const double xstep = (xmax - xmin) / width;
	//printf("Xstep: %f\n", xstep);

	const double ymin = data.ymin;
	const double ymax = data.ymax;
	const double ystep = (ymax - ymin) / height;
	//printf("Ystep: %f\n", ystep);

#ifdef USE_OPENMP
	#pragma omp parallel for 
#endif
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


struct ImageMetadata parseArgs(char* argv[]) {
    struct ImageMetadata metadata = {0, -2.0, 1.0, -1.5, 1.5, NULL};
    while (*argv) {
        if (strcmp(*argv, "-s") == 0) {
            argv++;
            int imageSize = (int) strtol(*argv, NULL, 10);
            if (imageSize > 0) {
                metadata.size = imageSize;
            } else {
                fprintf(stderr, "Image size must be greater than 0. (is %d)\n", imageSize);
            }
        } else if (strcmp(*argv, "-f") == 0) {
            argv++;
            metadata.path = *argv;
        } else if (strcmp(*argv, "-xmin") == 0) {
            argv++;
            metadata.xmin = strtod(*argv, NULL);
        } else if (strcmp(*argv, "-xmax") == 0) {
            argv++;
            metadata.xmax = strtod(*argv, NULL);
        } else if (strcmp(*argv, "-ymin") == 0) {
            argv++;
            metadata.ymin = strtod(*argv, NULL);
        } else if (strcmp(*argv, "-ymax") == 0) {
            argv++;
            metadata.ymax = strtod(*argv, NULL);
        } else {
            argv++;
        }
    }

    if (metadata.xmin > metadata.xmax) {
        double temp = metadata.xmin;
        metadata.xmin = metadata.xmax;
        metadata.xmax = temp;
    }

    if (metadata.ymin > metadata.ymax) {
        double temp = metadata.ymin;
        metadata.ymin = metadata.ymax;
        metadata.ymax = temp;
    }

    return metadata;
}

void printUsage() {
    printf("Usage: mandelbrot -f path [-s size | -xmin xmin | -xmax xmax | -ymin ymin | -ymax ymax]\n");
    printf("-f path:         Sets the path used to read and write the png image.\n");
    printf("                 If path does not exist a new image is created.\n");
    printf("-s size:         Sets the image size to size x size.\n");
    printf("                 Not used if the supplied path already contains an image.\n");
    printf("-[x|y][min|max]: Sets the coordinates for the picture. Defaults are:\n");
    printf("                 xmin: %f, xmax: %f, ymin: %f, ymax: %f\n", -2.0, 1.0, -1.5, 1.5);
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

    struct ImageMetadata data = parseArgs(argv);
    if (data.size > 0) printf("Image size: %d\n", data.size);
    if (data.path) printf("Image path: %s\n", data.path);

    SDL_Surface *image;
    if (data.path) {
        int imageSize = data.size > 0 ? data.size : DEFAULT_SIZE;

        image = IMG_Load(data.path);
        if (!image) {
            SDL_Log("IMG_Load error: %s\n", IMG_GetError());
            SDL_Log("Creating new image of size: %d\n", imageSize);

            image = SDL_CreateRGBSurfaceWithFormat(0, imageSize, imageSize, 32, SDL_PIXELFORMAT_ABGR8888);
        } else {
            data.size = image->w > image->h ? image->w : image->h;
        }
    } else {
        printUsage();
        return 3;
    }

    mandelbrot(image, data);

    Uint32 pixelformat = image->format->format;
    const char *formatName = SDL_GetPixelFormatName(pixelformat);
    printf("Format: %s\n", formatName);
    printf("BytesPerPixel: %d\n", image->format->BytesPerPixel);
    IMG_SavePNG(image, data.path);
    
    SDL_FreeSurface(image);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
