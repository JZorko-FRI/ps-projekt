#!/bin/bash

gcc kMeanSerial.c -lm -Wl,-rpath,./ -L./ -l:"libfreeimage.so.3" -o kMeanSerial
./kMeanSerial
