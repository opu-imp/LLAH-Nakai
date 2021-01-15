#!/bin/sh

llahdoc="/home/nakai/src/llahdoc_old/080620/temp/llahdoc"
dirs="/home/nakai/dirs/090307/100/"

deg_list=(90 60 45 30)
inv_list=(s a r)
nmin=10
nmax=10
mmin=7
mmax=10

invnum=0
while [ $invnum -le 2 ];
do
	n=$nmin
	while [ $n -le $nmax ];
	do
		m=$mmin
		while [ $m -le $n ];
#		while [ $m -le $mmax ];
		do
			d=2
			while [ $d -le 100 ];
			do
				$llahdoc -${inv_list[$invnum]} -x -h $dirs -n $n -m $m -d $d -u -y
				qnum=0
				while [ $qnum -le 3 ];
				do
					qpath="/home/nakai/image/3deg100/jpg/"${deg_list[$qnum]}"/*.jpg"
					opath="/home/nakai/src/llahdoc_old/080620/out/var_nm/"${inv_list[$invnum]}"/"${deg_list[$qnum]}"/"$n$m$d".txt"
					$llahdoc -e -h $dirs $qpath > $opath
					qnum=`expr $qnum + 1`
				done
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
	invnum=`expr $invnum + 1`
done
