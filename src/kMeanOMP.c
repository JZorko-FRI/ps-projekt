#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <stdbool.h>
#include "FreeImage.h"

int K = 64;  // number of clusters
int ITERATIONS = 20;  // readjustments of centroids

int THREADS = 1;

#define PATH_MAX 256
#define INPUT "../images/"
#define OUTPUT "../images/"

typedef struct RGB_t
{
    unsigned int *R;
    unsigned int *G;
    unsigned int *B;
    int centroidIndex;
} RGB;

bool *ignored;
RGB centroids;
int *centroidIndex;
unsigned char *imageIn;
int imageSize = 0;
int width = 0;
int height = 0;
int pitch = 0;

void initCentroids()
{
    #pragma omp parallel for
    for (int i = 0; i < K; i++)
    {
        int index = rand() % (width * height);  // TODO thread safe
        centroids.R[i] = imageIn[index * 4];
        centroids.G[i] = imageIn[index * 4 + 1];
        centroids.B[i] = imageIn[index * 4 + 2];
    }
}

void remodifyCentroids()
{
    int centroidPopularity[K];
    int centroidR[K];
    int centroidG[K];
    int centroidB[K];

    #pragma omp parallel for
    for (int i = 0; i < width * height; i++)
    {
        int index = centroidIndex[i];
        centroidPopularity[index]++;
        centroidR[index] += imageIn[i * 4];
        centroidG[index] += imageIn[i * 4 + 1];
        centroidB[index] += imageIn[i * 4 + 2];
    }

    #pragma omp parallel for
    for (int i = 0; i < K; i++)
    {
        if (centroidPopularity[i] != 0)
        {
            centroids.R[i] = centroidR[i] / centroidPopularity[i];
            centroids.G[i] = centroidG[i] / centroidPopularity[i];
            centroids.B[i] = centroidB[i] / centroidPopularity[i];
        }
        else {
            ignored[i] = true;
        }
    }
}

void findBestCentroidFor(int pixelIndex)
{
    int min = INT32_MAX;
    int min_i = 0;
    int razdalja = 0;
    int r = imageIn[pixelIndex * 4];
    int g = imageIn[pixelIndex * 4 + 1];
    int b = imageIn[pixelIndex * 4 + 2];


    for (int i = 0; i < K; i++)
    {
        if (ignored[i]) continue;

        int r2 = (centroids.R[i] - r) * (centroids.R[i] - r);
        int g2 = (centroids.G[i] - g) * (centroids.G[i] - g);
        int b2 = (centroids.B[i] - b) * (centroids.B[i] - b);

        razdalja = sqrt(r2 + g2 + b2);

        if (razdalja < min)
        {
            min = razdalja;
            min_i = i;
        }
    }
    centroidIndex[pixelIndex] = min_i;
}

void compressImage()
{
    initCentroids();

    for (int i = 0; i < ITERATIONS; i++)
    {
        // TODO multithread properly
        for (int j = 0; j < width * height; j++)
            findBestCentroidFor(j);

        if (i < ITERATIONS - 1)
            remodifyCentroids();
    }

    // TODO multithread properly
    for (int i = 0; i < width * height; i++)
    {
        imageIn[i * 4 + 0] = centroids.R[centroidIndex[i]];
        imageIn[i * 4 + 1] = centroids.G[centroidIndex[i]];
        imageIn[i * 4 + 2] = centroids.B[centroidIndex[i]];
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 4)
    {
        printf("Usage: ./kMeanOMP <K> <ITERATIONS> <THREADS>\n");
        return 1;
    }

    K = atoi(argv[1]);
    ITERATIONS = atoi(argv[2]);
    THREADS = atoi(argv[3]);

    omp_set_num_threads(THREADS);

    // Build input path
    char inputPath[PATH_MAX];
    strcpy(inputPath, INPUT);
    strcat(inputPath, "test.png"); // TODO change to arg

    // Prepare mask for ignored centroids
    ignored = (bool *)malloc(K * sizeof(bool));
    for (int i = 0; i < K; i++)
    {
        ignored[i] = false;
    }

    // printf("Loading image %s\n", inputPath);

    FIBITMAP *imageBitmap = FreeImage_Load(FIF_PNG, inputPath, 0);
    //Convert it to a 32-bit image
    FIBITMAP *imageBitmap32 = FreeImage_ConvertTo32Bits(imageBitmap);

    width = FreeImage_GetWidth(imageBitmap32);
    height = FreeImage_GetHeight(imageBitmap32);
    pitch = FreeImage_GetPitch(imageBitmap32);
    imageSize = height * pitch;

    //Preapare room for a raw data copy of the image
    imageIn = (unsigned char *)malloc(height * pitch * sizeof(unsigned char));
    centroidIndex = (int *)malloc(width * height * sizeof(int));

    //Extract raw data from the image
    FreeImage_ConvertToRawBits(imageIn, imageBitmap, pitch, 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
    centroids.R = (unsigned int *)malloc(K * sizeof(unsigned int));
    centroids.G = (unsigned int *)malloc(K * sizeof(unsigned int));
    centroids.B = (unsigned int *)malloc(K * sizeof(unsigned int));

    // Start timing execution
    clock_t begin = clock();

    // printf("Compressing image...\n");

    // Actual image compression
    compressImage();

    // Stop timing execution
    double time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC / THREADS;

    printf("OMP\tO2\t%dK\t%dI\t%dT\t%.0fms\n", K, ITERATIONS, THREADS, time_spent * 1000);

    // Build output path
    char outputPath[PATH_MAX];
    sprintf(outputPath, "%s%s_K%d_IT%d.png", OUTPUT, "stisnjena", K, ITERATIONS);  // TODO replace "stisnjena" with input name

    // printf("Saving image %s\n", outputPath);
    FIBITMAP *dst = FreeImage_ConvertFromRawBits(imageIn, width, height, pitch,
            32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

    FreeImage_Save(FIF_PNG, dst, outputPath, 0);

    //Free source image data
    FreeImage_Unload(imageBitmap32);
    FreeImage_Unload(imageBitmap);

    free(imageIn);

    return 0;
}
