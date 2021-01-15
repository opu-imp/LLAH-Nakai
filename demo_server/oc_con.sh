#!/bin/sh
# 登録スクリプト(Open Colledge用)
# 07/08/10

# 登録する画像のディレクトリ
#dbdir=/home/nakai/didb/jp/jpg/10000/
#dbdir=/home/nakai/didb/jp/1000/
#dbdir=/home/nakai/didb/test100_2/
dbdir=/home/nakai/didb/opencollege/
# ハッシュ等を保存するディレクトリ
#hashdir=./hash_jp0723_10000/
#hashdir=./hash_jp0723_1000/
hashdir=/home/nakai/dirs/oc07/

# 画像を用いるモード
./dcams -e -c $dbdir bmp -h $hashdir
