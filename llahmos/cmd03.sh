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
pdir="/home/nakai/dirs/point/""$num""/"
# 点ファイルの対応ファイル
dbcor="/home/nakai/dirs/dbcor/""$num""/dbcor.dat"
# 特徴点数ファイル
pn="/home/nakai/dirs/pnum/""$num""/pnum.txt"
# 登録画像の検索パス
rpath="/home/nakai/dirs/point/""$num""/*.dat"
# 離散化関数および補正定数の作成用データの検索パス
mpath="/home/nakai/dirs/point/10/*.dat"
# 検索質問の探索パス
# qpath="/home/nakai/image/3deg100/60/*.dat"
# 各種設定ファイルの格納ディレクトリ
ddir="/home/nakai/dirs/0421/""$num""/"
# 結果出力ディレクトリ
odir="out/"

inv=af
ch=a
n=7
m=6
disc=28
deg=60

opath="$odir""$deg""/""$inv""$n""$m""_""$disc""_""$num"".txt"
apath="$odir""$deg""/avg_""$inv""$n""$m""_""$num"".txt"
qpath="/home/nakai/image/3deg100/""$deg""/*.dat"

./dcams -S -I $ddir
# > "$opath"
# ../ares/ares "$opath" >> "$apath"
