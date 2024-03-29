#!/bin/bash

ITTTER=20

module load CUDA
gcc kMeanOpenCL.c -O2 -lm -fopenmp -lOpenCL -Wl,-rpath,./ -L./ -l:"libfreeimage.so.3" -o kMeanOpenCL

for K in 16 32 64 128 256
do
    for i in {1..10}
    do
        echo -e "\tTrial $i"
        srun -n1 -G1 --reservation=fri kMeanOpenCL $K $ITTTER >> ocl.tsv &
    done
done

#Pretvorba 200 framov videa
# for f in ../images/video_in/*.png 
# do
#     arrIN=(${f//'/'/ })
#     echo ${arrIN[3]}                  
#     srun -n1 -G1 --reservation=fri kMeanOpenCL ${arrIN[3]}
# done