#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "FreeImage.h"

#define K 64  // number of clusters
#define ITERATIONS 20  //

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

RGB centroids;
int *centroidIndex;
unsigned char *imageIn;
int imageSize = 0;
int width = 0;
int height = 0;
int pitch = 0;

void initCentroids()
{
    for (int i = 0; i < K; i++)
    {
        int index = rand() % (width * height);
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

    for (int i = 0; i < width * height; i++)
    {
        int index = centroidIndex[i];
        centroidPopularity[index]++;
        centroidR[index] += imageIn[i * 4];
        centroidG[index] += imageIn[i * 4 + 1];
        centroidB[index] += imageIn[i * 4 + 2];
    }

    for (int i = 0; i < K; i++)
    {
        centroids.R[i] = centroidR[i] / centroidPopularity[i];
        centroids.G[i] = centroidG[i] / centroidPopularity[i];
        centroids.B[i] = centroidB[i] / centroidPopularity[i];
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

int main(void)
{
    // Build input path
    char inputPath[PATH_MAX];
    strcpy(inputPath, INPUT);
    strcat(inputPath, "test.png"); // TODO change to arg

    // TODO K and INTERATIONS as args

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

    initCentroids();

    for (int i = 0; i < ITERATIONS; i++)
    {
        for (int j = 0; j < width * height; j++)
            findBestCentroidFor(j);

        if (i < ITERATIONS - 1)
            remodifyCentroids();
    }

    for (int i = 0; i < width * height; i++)
    {
        imageIn[i * 4 + 0] = centroids.R[centroidIndex[i]];
        imageIn[i * 4 + 1] = centroids.G[centroidIndex[i]];
        imageIn[i * 4 + 2] = centroids.B[centroidIndex[i]];
    }

    // Stop timing execution
    double time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;

    // printf("Processed in %.0fms\n", time_spent * 1000);
    printf("Serial\tO2\t%dK\t%dI\t%dT\t%.0fms\n", K, ITERATIONS, 1, time_spent * 1000);

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
