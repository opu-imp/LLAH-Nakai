#!/bin/sh
# origÇÃÇ†ÇÈÇ‡ÇÃÇæÇØÇ≈é¿çs

# Dir name setting

origdir=/home/nakai/jutaku/600dpi/orig/
annodir=/home/nakai/jutaku/600dpi/
outdir=clcor/
annex=./annex

n=5
m=5
disc=50
cluster=5

for path in `find $origdir -name "*.jpg" -maxdepth 1`
do
	file=`basename $path .jpg`
	echo $file
# scan
	origfname=$annodir$file"0.jpg"
	count=1
	while [ $count -le 3 ];
	do
		annofname=$annodir$file$count".jpg"

		$annex "-n"$n "-m"$m "-d"$disc "-c"$cluster -E 10000000000 $origfname $annofname
		count=`expr $count + 1`
	done
done
