#!/bin/bash

# Test Serial

for k in 1 2 4 8 16 32 64 128 256
do
    for iter in 1 2 4 8 16 32
    do
        srun \
          --reservation=fri \
          --constraint=AMD \
          -c 1 -n 10 -Q \
          -o serial.tsv \
          --open-mode=append \
          kMeanSerial $k $iter &
    done
done

squeue --me
