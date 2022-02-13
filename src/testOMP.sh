#!/bin/bash

# Test OMP

# Number of threads to test
# for t in 1 2 4 8 12 16
# for t in 24 32 48 64
for t in 1 2 4 8
do
    for k in 64
    do
        for iter in 20
        do
            srun \
            --reservation=fri \
            --constraint=AMD \
            -c $t -n 5 -Q \
            -o ompOptimization.tsv \
            --open-mode=append \
            kMeanOMP $k $iter $t &
        done
    done
done

squeue --me
