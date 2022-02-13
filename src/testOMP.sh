#!/bin/bash

# Test OMP

# Number of threads to test
for t in 1 2 4 8 12 16 24 32 48 64
do
    for k in 64
    do
        for iter in 20
        do
            srun \
            --reservation=fri \
            --constraint=AMD \
            -c $t -n 10 -Q \
            -o omp.tsv \
            --open-mode=append \
            kMeanSerial $k $iter $t &
        done
    done
done

squeue --me
