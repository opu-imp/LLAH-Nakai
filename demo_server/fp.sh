#!/bin/sh

# 登録する画像の点ファイルのディレクトリ
pdir="/home/nakai/dirs/point/opencollege/"
# 点ファイルの対応ファイル
dbcor="/home/nakai/dirs/dbcor/opencollege/dbcor.dat"
# 特徴点数ファイル
pn="/home/nakai/dirs/pnum/opencollege/pnum.txt"
# 登録画像の検索パス
rpath="/home/nakai/dirs/point/opencollege/*.dat"
# 離散化関数および補正定数の作成用データの検索パス
mpath="/home/nakai/dirs/point/opencollege/*.dat"
# 検索質問の探索パス
# qpath="/home/nakai/image/3deg100/60/*.dat"
# 各種設定ファイルの格納ディレクトリ
ddir="/home/nakai/dirs/opencollege/"
# 結果出力ディレクトリ
odir="out/"

inv=af
ch=a
n=7
m=6
#disc=28
disc=15
deg=60

#opath="$odir""$deg""/""$inv""$n""$m""_""$disc""_""$num"".txt"
#apath="$odir""$deg""/avg_""$inv""$n""$m""_""$num"".txt"
#qpath="/home/nakai/image/3deg100/""$deg""/*.dat"

./dcams -"$ch"cf $pdir $dbcor $pn -n"$n" -m"$m" -d"$disc" -D "$mpath" -I $ddir -P "$mpath"
