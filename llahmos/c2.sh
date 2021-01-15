#!/bin/sh

./dcams -n 9 -m 8 -x
echo n=9, m=8, normal, all >> out0425_2.txt
./dcams -e /home/nakai/image/3deg100/60/*.dat >> out0425_2.txt

./dcams -n 9 -m 8 -x -z
echo n=9, m=8, compress, all >> out0425_2.txt
./dcams -e /home/nakai/image/3deg100/60/*.dat >> out0425_2.txt

./dcams -n 9 -m 8 -x -y
echo n=9, m=8, normal, rot >> out0425_2.txt
./dcams -e -y /home/nakai/image/3deg100/60/*.dat >> out0425_2.txt

./dcams -n 9 -m 8 -x -y -z
echo n=9, m=8, compress, rot >> out0425_2.txt
./dcams -e -y /home/nakai/image/3deg100/60/*.dat >> out0425_2.txt

