#!/bin/sh

# 登録画像数（引数の値にセット 引数なしや無効な値なら100）
if [ $# -ge 1 ];
then
	case $1 in
		100 | 1000 | 10000)	num=$1;;
		20000)	num=20596;;
		*)	num=100;;
	esac
else
	num=100
fi
#num=10000
# 登録する画像の点ファイルのディレクトリ
pdir="/home/nakai/dirs/point/icpr06_""$num""/"
# 点ファイルの対応ファイル
dbcor="/home/nakai/dirs/dbcor/icpr06_""$num""/dbcor.dat"
# 特徴点数ファイル
pn="/home/nakai/dirs/pnum/icpr06_""$num""/pnum.txt"
# 登録画像の検索パス
rpath="/home/nakai/dirs/point/icpr06_""$num""/*.dat"
# 離散化関数および補正定数の作成用データの検索パス
mpath="/home/nakai/dirs/point/icpr06_10/*.dat"
# 検索質問の探索パス
# qpath="/home/nakai/image/3deg100/60/*.dat"
# 各種設定ファイルの格納ディレクトリ
ddir="/home/nakai/dirs/icpr06/""$num""/"
# 結果出力ディレクトリ
odir="out/"

inv=af
ch=a
n=7
m=6
disc=28
deg=60
v=6

opath="$odir""$deg""/""$inv""$n""$m""_""$disc""_""$num"".txt"
apath="$odir""$deg""/avg_""$inv""$n""$m""_""$num"".txt"
qpath="/home/nakai/image/3deg100/""$deg""/*.dat"

#./dcams -S -I $ddir
./dcams -S -I $ddir -v$v
# > "$opath"
# ../ares/ares "$opath" >> "$apath"
