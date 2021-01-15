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
# 各種設定ファイルの格納ディレクトリ
ddir="/home/nakai/dirs/jp200/"
# 結果出力ディレクトリ
odir="out/"

./dcams -S -I $ddir
