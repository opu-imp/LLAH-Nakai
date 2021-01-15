#!/bin/sh
# 検索スクリプト（日本語用）
# 07/07/23

# ハッシュ等を保存するディレクトリ
#hashdir=./hash_jp0723_1000/
#hashdir=./hash_jp0723_1000/
# 検索質問のディレクトリ（以下に90/*.jpg等と続く）
#quedir=/home/nakai/image/3deg100jp/
#quedir=/home/nakai/image/jp/0724small4/
#quedir=/home/nakai/image/3deg100/jpg/

outfile=out0724100.txt
quedir=/home/nakai/image/jp/0724/

echo no_area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100_2/ "$quedir"*.JPG >> $outfile
echo area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100/ "$quedir"*.JPG >> $outfile
echo no_area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000_2/ "$quedir"*.JPG >> $outfile
echo area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000/ "$quedir"*.JPG >> $outfile
echo no_area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000_2/ "$quedir"*.JPG >> $outfile
echo area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000/ "$quedir"*.JPG >> $outfile

outfile=out072475.txt
quedir=/home/nakai/image/jp/0724small75/

echo no_area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100_2/ "$quedir"*.JPG >> $outfile
echo area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100/ "$quedir"*.JPG >> $outfile
echo no_area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000_2/ "$quedir"*.JPG >> $outfile
echo area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000/ "$quedir"*.JPG >> $outfile
echo no_area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000_2/ "$quedir"*.JPG >> $outfile
echo area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000/ "$quedir"*.JPG >> $outfile

outfile=out072460.txt
quedir=/home/nakai/image/jp/0724small60/

echo no_area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100_2/ "$quedir"*.JPG >> $outfile
echo area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100/ "$quedir"*.JPG >> $outfile
echo no_area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000_2/ "$quedir"*.JPG >> $outfile
echo area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000/ "$quedir"*.JPG >> $outfile
echo no_area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000_2/ "$quedir"*.JPG >> $outfile
echo area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000/ "$quedir"*.JPG >> $outfile

outfile=out072450.txt
quedir=/home/nakai/image/jp/0724small50/

echo no_area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100_2/ "$quedir"*.JPG >> $outfile
echo area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100/ "$quedir"*.JPG >> $outfile
echo no_area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000_2/ "$quedir"*.JPG >> $outfile
echo area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000/ "$quedir"*.JPG >> $outfile
echo no_area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000_2/ "$quedir"*.JPG >> $outfile
echo area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000/ "$quedir"*.JPG >> $outfile

outfile=out072425.txt
quedir=/home/nakai/image/jp/0724small25/

echo no_area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100_2/ "$quedir"*.JPG >> $outfile
echo area reg 100 >> $outfile
./dcams -J -e -h ./hash_jp0723_100/ "$quedir"*.JPG >> $outfile
echo no_area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000_2/ "$quedir"*.JPG >> $outfile
echo area reg 1000 >> $outfile
./dcams -J -e -h ./hash_jp0723_1000/ "$quedir"*.JPG >> $outfile
echo no_area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000_2/ "$quedir"*.JPG >> $outfile
echo area reg 10000 >> $outfile
./dcams -J -e -h ./hash_jp0723_10000/ "$quedir"*.JPG >> $outfile
