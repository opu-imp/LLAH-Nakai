#!/bin/sh

annex=./annex
scandir=/home/nakai/jutaku/prmu0703/
outdir=./mono/

cluster=2
threshold=5000
search_neighbor=4
min_area=10
n=5
m=5
disc=50
max_con=10000000000

origfname=a01010210.jpg
annofname=a01010211.jpg

for path in `find $scandir -name "*0.jpg" -maxdepth 1`
do
	file=`basename $path 0.jpg`
	echo $file
	origfname=$file"0.jpg"
	annofname=$file"1.jpg"

	$annex "-n"$n "-m"$m "-d"$disc "-c"$cluster "-E"$max_con "-H"$threshold "-N"$search_neighbor "-o"$outdir $origfname $annofname
done
