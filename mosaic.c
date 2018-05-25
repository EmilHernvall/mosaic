#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <gd.h>
#include <math.h>
#include <unistd.h>

#include "util.h"
#include "vector.h"

typedef struct image_ {
    char *path;
    int index;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} image;

// Crops an image to a square of a certain size and calculates
// the average color of the crop.
image *create_tile(const char *source_dir, const char *target_dir,
        int size, int index, const char *current, int verbose)
{
    int len;
    char path[PATH_MAX];
    FILE *fh;
    gdImage *img, *img2;
    int oldWidth, oldHeight;
    int copyWidth, copyHeight;
    int color;
    int x,y;
    int r, g, b;
    int c;
    image *data;

    len = strlen(source_dir);

    // Concatenate the path with the filename,
    // and insert a / if necessary.
    if (source_dir[len] != '/') {
        sprintf(path, "%s/%s", source_dir, current);
    } else {
        sprintf(path, "%s%s", source_dir, current);
    }

    if (verbose) {
        printf("Processing %s:\n", path);
    }

    fh = fopen(path, "r");
    if (fh == NULL) {
        perror("fopen");
        exit(1);
    }

    img = gdImageCreateFromJpeg(fh);
    fclose(fh);

    oldWidth = gdImageSX(img);
    oldHeight = gdImageSY(img);

    img2 = gdImageCreateTrueColor(size, size);

    if (verbose) {
        printf("Original size: %dx%d\n", oldWidth, oldHeight);
        printf("New size: %dx%d\n", size, size);
    }

    if (oldWidth > oldHeight) {
        copyWidth = oldHeight;
        x = (oldWidth - copyWidth) / 2;

        copyHeight = oldHeight;		
        y = 0;

        if (verbose) {
            printf("Cropping at %d, %d\n", x, y);
            printf("Crop size: %dx%d\n", copyWidth, copyHeight);
        }
    } 
    else if (oldWidth < oldHeight) {
        copyWidth = oldWidth;
        x = 0;

        copyHeight = oldWidth;
        y = (oldHeight - copyHeight) / 2;

        if (verbose) {
            printf("Cropping at %d, %d\n", x, y);
            printf("Crop size: %dx%d\n", copyWidth, copyHeight);
        }
    }
    else {
        copyWidth = oldWidth;
        x = 0;

        copyHeight = oldHeight;
        y = 0;
    }

    gdImageCopyResampled(img2, img, 0, 0, x, y, size, size, copyWidth, copyHeight);

    len = strlen(target_dir);
    if (target_dir[len] != '/') {
        sprintf(path, "%s/%d.jpg", target_dir, index);
    } else {
        sprintf(path, "%s%d.jpg", target_dir, index);
    }

    if (verbose) {
        printf("Writing to %s\n\n", path);
    }

    fh = fopen(path, "w");
    if (fh == NULL) {
        perror("fopen");
        exit(1);
    }

    gdImageJpeg(img2, fh, 90);
    fclose(fh);

    // Calculate average color of crop
    r = 0;
    g = 0;
    b = 0;
    for (x = 0; x < size; x++) {
        for (y = 0; y < size; y++) {
            color = gdImageGetPixel(img2, x, y);

            r += gdImageRed(img2, color);
            g += gdImageGreen(img2, color);
            b += gdImageBlue(img2, color);
        }
    }

    r /= size*size;
    g /= size*size;
    b /= size*size;

    gdImageDestroy(img);
    gdImageDestroy(img2);	

    data = (image*)xmalloc(sizeof(image));
    data->path = xstrdup(path);
    data->index = index;
    data->r = r;
    data->g = g;
    data->b = b;

    return data;
}

// Load a tile, ensure that it has the specified size,
// and calculate average color.
image *open_tile(const char *target_dir, int size, 
        int index, const char *current, int verbose)
{
    int len;
    char path[PATH_MAX];
    FILE *fh;
    gdImage *img;
    int width, height;
    int color;
    int x,y;
    int r, g, b;
    image *data;

    len = strlen(target_dir);
    if (target_dir[len] != '/') {
        sprintf(path, "%s/%s", target_dir, current);
    } else {
        sprintf(path, "%s%s", target_dir, current);
    }

    if (verbose) {
        printf("Loading %s\n", path);
    }

    fh = fopen(path, "r");
    if (fh == NULL) {
        perror("fopen");
        exit(1);
    }

    img = gdImageCreateFromJpeg(fh);
    fclose(fh);

    width = gdImageSX(img);
    height = gdImageSY(img);

    if (width != size || height != size) {
        fprintf(stderr, 
                "Tiles directory contains images that does not match \n"
                "the specified size (%dx%d). To change this size, use \n"
                "the -s option.\n", size, size);
        exit(1);
    }

    // Calculate average color
    r = 0;
    g = 0;
    b = 0;
    for (x = 0; x < size; x++) {
        for (y = 0; y < size; y++) {
            color = gdImageGetPixel(img, x, y);

            r += gdImageRed(img, color);
            g += gdImageGreen(img, color);
            b += gdImageBlue(img, color);
        }
    }

    r /= size*size;
    g /= size*size;
    b /= size*size;

    gdImageDestroy(img);

    data = (image*)xmalloc(sizeof(image));
    data->path = xstrdup(path);
    data->index = index;
    data->r = r;
    data->g = g;
    data->b = b;

    return data;
}

// Get the average color of the square of size d*d
// at pos x,y. Return it encoded as a single integer.
int get_average(gdImage *img, int xs, int ys, int d)
{
    int x, y;
    unsigned int r, g, b;
    int color;

    r = 0;
    g = 0;
    b = 0;
    for (x = xs; x < xs + d; x++) {
        for (y = ys; y < ys + d; y++) {
            color = gdImageGetPixel(img, x, y);

            r += gdImageRed(img, color);
            g += gdImageGreen(img, color);
            b += gdImageBlue(img, color);
        }
    }

    r /= d*d;
    g /= d*d;
    b /= d*d;

    return (b << 16) | (g << 8) | r;
}

// Find the tile that most closely matches a certain color.
// Use a simple euclidian distance calculation to find
// the minimum.
int find_closest(vector *images, int color)
{
    int i;
    int count;
    int r, g, b;
    image *img;
    double distance;
    double minDistance;
    int minMatch;

    r = color & 0xFF;
    g = (color >> 8) & 0xFF;
    b = (color >> 16) & 0xFF;

    count = vector_count(images);
    minDistance = sqrt(3 * 0xFF * 0xFF);
    for (i = 0; i < count; i++) {
        img = vector_get(images, i);

        distance = sqrt(pow(img->r - r, 2) + pow(img->g - g, 2) + pow(img->b - b, 2));
        if (distance < minDistance) {
            minDistance = distance;
            minMatch = i;
        }
    }

    return minMatch;
}

// Build an image mosaic from a set of imported tiles.
void build_mosaic(const char *source_image, const char *out_image, int d, 
        int size, vector *images, int verbose)
{
    FILE *fh;
    gdImage *img, *img2, *img3;
    int width, height;
    int newWidth, newHeight;
    int x, y;
    int x2, y2;
    int c;
    int match;
    image *data;

    fh = fopen(source_image, "r");
    if (fh == NULL) {
        perror("fopen");
        exit(1);
    }

    img = gdImageCreateFromJpeg(fh);
    fclose(fh);

    width = gdImageSX(img);
    height = gdImageSY(img);

    newWidth = width / d * size;
    newHeight = height / d * size;

    printf("Creating new image of size %dx%d.\n", newWidth, newHeight);

    img2 = gdImageCreateTrueColor(newWidth, newHeight);

    for (x = 0, x2 = 0; x < width; x += d, x2 += size) {
        if (verbose) {
            printf("Processing row %d of %d.\n", x / d, width / d);
        }

        for (y = 0, y2 = 0; y < height; y += d, y2 += size) {
            c = get_average(img, x, y, d);
            match = find_closest(images, c);
            data = vector_get(images, match);

            fh = fopen(data->path, "r");
            if (fh == NULL) {
                perror("fopen");
                exit(1);
            }

            img3 = gdImageCreateFromJpeg(fh);
            fclose(fh);

            gdImageCopyResampled(img2, img3, x2, y2, 0, 0, size, size, size, size);
            gdImageDestroy(img3);
        }
    }

    printf("Done. Saving image: %s\n", out_image);

    fh = fopen(out_image, "w");
    if (fh == NULL) {
        perror("fopen");
        exit(1);	
    }

    gdImageJpeg(img2, fh, 50);
    fclose(fh);

    gdImageDestroy(img2);
    gdImageDestroy(img);
}

void print_usage(FILE *stream, const char *program_name)
{
    fprintf(stream, "Usage: %s options infile outfile\n", program_name);
    fprintf(stream,
            "   -h  --help          Display this usage information.\n"
            "   -v  --verbose       Output a lot of internal stuff.\n"
            "   -c  --convert dir   Convert all images in dir to tiles suited\n"
            "                       for using as mosaic pieces.\n"
            "   -t  --tiles dir    Directory containing tiles, or the target for\n"
            "                       the conversion.\n"
            "   -d  --delta n       The size of the square of pixels to replace.\n"
            "   -s  --size n        The size of the tiles.\n"
            "\n"
            "Generating a completely new directory:\n"
            "mosaic -c my_photos -t tiles_dir in.jpg out.jpg\n"
            "\n"
            "Using an existing directory:\n"
            "mosaic -t tiles_dir in.jpg out.jpg\n"
            "\n"
            "One time generation:\n"
            "mosaic -c my_photos in.jpg out.jpg\n"
            "\n"
            "By Emil Hernvall <aderyn@gmail.com>\n");
}

int main(int argc, char **argv)
{
    int action = 0;
    int verbose = 0;
    const char *source_image = NULL;
    const char *out_image = NULL;
    int d = 25;
    const char *source_dir = NULL;
    const char *target_dir = NULL;
    int size = 50;

    int next_option;
    const char *short_options = "hvc:t:d:s:";
    const struct option long_options[] = {
        { "help", 0, NULL, 'h' },
        { "verbose", 0, NULL, 'v' },
        { "convert", 1, NULL, 'c' },
        { "tiles", 1, NULL, 't' },
        { "delta", 1, NULL, 'd' },
        { "size", 1, NULL, 's' },
        { NULL, 0, NULL, 0 } };

    DIR *dir;
    struct dirent *entry;
    char *ext;
    int i;
    vector images;
    image *img;

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        switch (next_option) {
            case 'h':
                print_usage(stdout, argv[0]);
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 'c':
                source_dir = optarg;
                break;
            case 't':
                target_dir = optarg;
                break;
            case 'd':
                d = atoi(optarg);
                break;
            case 's':
                size = atoi(optarg);
                break;
            case -1:
                break;
            default:
                abort();
        }
    } while (next_option != -1);

    if (optind + 1 > argc) {
        print_usage(stderr, argv[0]);
        return 1;
    }

    source_image = argv[optind];
    out_image = argv[optind + 1];

    if (source_dir != NULL && target_dir != NULL) {
        printf("Building new tileset.\n\n");
        action = 1;
    }
    else if (target_dir != NULL) {
        printf("Using an existing tileset.\n\n");
        action = 2;
    }
    else if (source_dir != NULL) {
        printf("Generating a temporary tileset.\n\n");
        action = 3;
    }
    else {
        fprintf(stderr, "No valid action specified!\n");
        print_usage(stderr, argv[0]);
        return 1;
    }

    if (action == 3) {
        target_dir = ".tileset-temp/";
        if (mkdir(target_dir, 0777)) {
            fprintf(stderr, "Failed to create temporary directory %s.\n", target_dir);
            return 0;
        }
    }

    printf("Source image: %s\n", source_image);
    printf("Out image: %s\n", out_image);
    printf("Delta: %d\n", d);
    printf("Source directory: %s\n", source_dir);
    printf("Target directory: %s\n", target_dir);
    printf("Pieces size: %dx%d\n", size, size);
    printf("\n");

    vector_init(&images);

    if (action == 2) {
        printf("Loading tiles.\n");
        dir = opendir(target_dir);
    } else {
        printf("Creating tiles.\n");
        dir = opendir(source_dir);
    }

    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    i = 0;
    while ((entry = readdir(dir)) != NULL) {
        ext = strchr(entry->d_name, '.');
        if (strcasecmp(ext, ".jpg") != 0) {
            continue;
        }

        if (action == 2) {
            img = open_tile(target_dir, size, i, entry->d_name, verbose);
        } else {
            img = create_tile(source_dir, target_dir, size, i, entry->d_name, verbose);
        }

        vector_add(&images, img);

        i++;
    }

    closedir(dir);

    build_mosaic(source_image, out_image, d, size, &images, verbose);

    if (action == 3) {
        printf("Cleaning up temporary files.\n");
        for (i = 0; i < vector_count(&images); i++) {
            image *img = vector_get(&images, i);
            unlink(img->path);
        }
        rmdir(target_dir);
    }

    return 0;
}
