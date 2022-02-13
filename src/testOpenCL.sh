#!/bin/bash


module load CUDA
gcc kMeanOpenCL.c -O2 -lm -fopenmp -lOpenCL -Wl,-rpath,./ -L./ -l:"libfreeimage.so.3" -o kMeanOpenCL
srun -n1 -G1 --reservation=fri kMeanOpenCL