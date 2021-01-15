#!/bin/sh

# 登録する画像のディレクトリ
dbdir=/home/nakai/didb/test1000/

#./dcams -c $dbdir bmp
# 既存のpoint.datを用いるモード
# ./dcams -x
# ハッシュ圧縮モード
./dcams -x -y
