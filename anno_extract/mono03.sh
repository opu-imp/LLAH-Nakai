#!/bin/sh

annex=./annex
scandir=/home/nakai/jutaku/prmu0703/
outdir=./out070218/

cluster=2
threshold=5000
search_neighbor=4
min_area=10
n=5
m=5
disc=50
max_con=10000000000

origfname=a01010130.jpg
annofname=a01010131.jpg


$annex "-n"$n "-m"$m "-d"$disc "-c"$cluster "-E"$max_con "-H"$threshold "-N"$search_neighbor "-o"$outdir $origfname $annofname
