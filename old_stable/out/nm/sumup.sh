#!/bin/sh

outdir=/home/nakai/src/llahdoc_old/080620/out/nm/

#deg_list=(90 60 45 30)
deg_list=(60)
nmin=6
nmax=10
mmin=6

qnum=0
#while [ $qnum -le 3 ];
while [ $qnum -le 0 ];
do
	n=$nmin
	while [ $n -le $nmax ];
	do
		m=$mmin
		while [ $m -le $n ];
		do
			d=2
			while [ $d -le 100 ];
			do
				txtpath=$outdir${deg_list[$qnum]}"/"$n$m$d".txt"
				echo $txtpath
				
				if [ $d -le 9 ];
				then
					d=`expr $d + 1`
				else
					d=`expr $d + 10`
				fi
			done
			m=`expr $m + 1`
		done
		n=`expr $n + 1`
	done
	qnum=`expr $qnum + 1`
done
