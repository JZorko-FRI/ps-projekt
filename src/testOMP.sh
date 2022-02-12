#!/bin/bash

# Test OMP

# Number of threads to test
for t in 1 2 4 8 12 16 24 32 48 64
do
    # Repeat each configuration multiple times
    echo Testing $t threads
    for i in {1..10}
    do
        echo -e "\tTrial $i"
        srun --reservation=fri --constraint=AMD -c $t kMeanOMP 256 32 $t >> omp.tsv &
    done
done
echo Done
