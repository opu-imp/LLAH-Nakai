#!/bin/sh

# 登録画像の検索パス
rpath="/home/nakai/didb/jp/test200/*.bmp"
# 登録する画像の点ファイルのディレクトリ
pdir="/home/nakai/dirs/point/jp200/"
# 点ファイルの対応ファイル
dbcor="/home/nakai/dirs/dbcor/jp200/dbcor.dat"
# 特徴点数ファイル
pn="/home/nakai/dirs/pnum/jp200/pnum.txt"
# 開始ナンバー（デフォルトは0）
start=0

./dcams -JG "$rpath" $pdir $dbcor $pn $start
