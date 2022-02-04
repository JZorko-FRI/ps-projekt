#!/bin/bash

gcc kMeanSerial.c -lm -Wl,-rpath,./ -L./ -l:"../bin/libfreeimage.so.3" -o kMeanSerial
./kMeanSerial
