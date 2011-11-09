#!/bin/bash

echo -ne "\rled 1\rled 0\rgfxinit 0\r" | ./client /dev/ttyUSB1
echo -ne "\rteleled 0 1\rteleled 0 0\rgfxinit 0\r" | ./client /dev/ttyUSB1

for Y in `seq 0 8`
do
    for X in `seq 0 16`
    do
        echo -ne "gfxplot 0 $((X*8)) $Y F0F0F0F00F0F0F0F\r" | ./client /dev/ttyUSB1
    done
done

