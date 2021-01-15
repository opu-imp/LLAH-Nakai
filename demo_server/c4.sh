#!/bin/sh
# 登録スクリプト（日本語用）
# 07/07/23

# 登録する画像のディレクトリ
dbdir=/home/nakai/didb/jp/jpg/10000/
#dbdir=/home/nakai/didb/jp/1000/
#dbdir=/home/nakai/didb/test100_2/
# ハッシュ等を保存するディレクトリ
hashdir=./hash_jp0723_10000_2/
#hashdir=./hash_jp0723_1000/

# 画像を用いるモード
#./dcams -J -e -c $dbdir jpg -h $hashdir >> out0724_2.txt
#./dcams -e -c $dbdir bmp -h $hashdir >> out0723.txt
# 既存のpoint.datを用いるモード
#./dcams -J -u -e -x -h $hashdir >> out0724.txt
./dcams -J -e -x -h $hashdir >> out0724_2.txt
#./dcams -u -x -h $hashdir >> out0723.txt
