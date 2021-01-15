#!/bin/sh

llahdoc="/home/nakai/src/llahdoc_old/080620/llahdoc"
dirs_dir="/home/nakai/dirs/090307/"

deg_list=(90 60 45 30)
inv_list=(s a r)
d_list0=(3 3 2 2)
d_list1=(4 4 4 3)
d_list2=(7 7 7 5)
n=8
m=7

invnum=0
pages=100
while [ $pages -le 10000 ];
do
	dirs=$dirs_dir$pages"/"
	d=${d_list0[$invnum]}
	$llahdoc -${inv_list[$invnum]} -x -h $dirs -n $n -m $m -d $d -u -y
	qnum=0
	while [ $qnum -le 3 ];
	do
		qpath="/home/nakai/image/3deg100/jpg/"${deg_list[$qnum]}"/*.jpg"
		opath="/home/nakai/src/llahdoc_old/080620/out/var_pages/"${inv_list[$invnum]}"/"${deg_list[$qnum]}"/"$n$m$d$pages".txt"
		$llahdoc -e -h $dirs $qpath > $opath
		qnum=`expr $qnum + 1`
	done
	pages=`expr $pages \* 10`
done

invnum=1
pages=100
while [ $pages -le 10000 ];
do
	dirs=$dirs_dir$pages"/"
	d=${d_list1[$invnum]}
	$llahdoc -${inv_list[$invnum]} -x -h $dirs -n $n -m $m -d $d -u -y
	qnum=0
	while [ $qnum -le 3 ];
	do
		qpath="/home/nakai/image/3deg100/jpg/"${deg_list[$qnum]}"/*.jpg"
		opath="/home/nakai/src/llahdoc_old/080620/out/var_pages/"${inv_list[$invnum]}"/"${deg_list[$qnum]}"/"$n$m$d$pages".txt"
		$llahdoc -e -h $dirs $qpath > $opath
		qnum=`expr $qnum + 1`
	done
	pages=`expr $pages \* 10`
done

invnum=2
pages=100
while [ $pages -le 10000 ];
do
	dirs=$dirs_dir$pages"/"
	d=${d_list2[$invnum]}
	$llahdoc -${inv_list[$invnum]} -x -h $dirs -n $n -m $m -d $d -u -y
	qnum=0
	while [ $qnum -le 3 ];
	do
		qpath="/home/nakai/image/3deg100/jpg/"${deg_list[$qnum]}"/*.jpg"
		opath="/home/nakai/src/llahdoc_old/080620/out/var_pages/"${inv_list[$invnum]}"/"${deg_list[$qnum]}"/"$n$m$d$pages".txt"
		$llahdoc -e -h $dirs $qpath > $opath
		qnum=`expr $qnum + 1`
	done
	pages=`expr $pages \* 10`
done
