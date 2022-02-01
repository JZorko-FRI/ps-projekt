#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "FreeImage.h"

#define K 64
#define ITTERATIONS 20

typedef struct RGB_t {
	unsigned int* R;
	unsigned int* G;
	unsigned int* B;
    int centroidIndex;
} RGB;

RGB centroids;
int* centroidIndex;
unsigned char *imageIn;
int imageSize = 0;
int width = 0;
int height = 0;
int pitch = 0;


void initCentroids()
{
    for(int i = 0; i < K; i++)
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
    
    for(int i = 0; i < width * height;i++)
    {
        int index = centroidIndex[i];
        centroidPopularity[index]++;
        centroidR[index] += imageIn[i * 4];
        centroidG[index] += imageIn[i * 4 + 1];
        centroidB[index] += imageIn[i * 4 + 2];
    }
    for(int i = 0; i < K;i++)
    {
        centroids.R[i] = centroidR[i]/centroidPopularity[i];
        centroids.G[i] = centroidG[i]/centroidPopularity[i];
        centroids.B[i] = centroidB[i]/centroidPopularity[i];
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
    
    for(int i = 0; i < K; i++)
    {
        razdalja = sqrt((centroids.R[i] - r) * (centroids.R[i] - r) + (centroids.G[i] - g) * (centroids.G[i] - g) + (centroids.B[i] - b) * (centroids.B[i] - b));
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
    FIBITMAP *imageBitmap = FreeImage_Load(FIF_PNG, "images/test.png", 0);
	//Convert it to a 32-bit image
    FIBITMAP *imageBitmap32 = FreeImage_ConvertTo32Bits(imageBitmap);

    width  = FreeImage_GetWidth(imageBitmap32);
	height = FreeImage_GetHeight(imageBitmap32);
	pitch  = FreeImage_GetPitch(imageBitmap32);
    imageSize = height * pitch;

    //Preapare room for a raw data copy of the image
    imageIn = (unsigned char *) malloc (height * pitch * sizeof(unsigned char));
    centroidIndex = (int *) malloc (width * height * sizeof(int));
    
    
    //Extract raw data from the image
	FreeImage_ConvertToRawBits(imageIn, imageBitmap, pitch, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
    centroids.R = (unsigned int*) malloc (K * sizeof(unsigned int));
    centroids.G = (unsigned int*) malloc (K * sizeof(unsigned int));
    centroids.B = (unsigned int*) malloc (K * sizeof(unsigned int));
    
    initCentroids();

    for(int i = 0; i < ITTERATIONS;i++)
    {
        for(int j = 0; j < width * height; j++)
            findBestCentroidFor(j);
        
        if(i < ITTERATIONS - 1)remodifyCentroids();
    }


    for(int i = 0; i < width * height;i++)
    {
        imageIn[i * 4 + 0] = centroids.R[centroidIndex[i]];
        imageIn[i * 4 + 1] = centroids.G[centroidIndex[i]];
        imageIn[i * 4 + 2] = centroids.B[centroidIndex[i]];  
    }
    

    FIBITMAP *dst = FreeImage_ConvertFromRawBits(imageIn, width, height, pitch,
		32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
	FreeImage_Save(FIF_PNG, dst, "stisnjena64.png", 0);

    //Free source image data
	FreeImage_Unload(imageBitmap32);
	FreeImage_Unload(imageBitmap);  

	free(imageIn);

    return 0;
}
