#!/bin/sh
# �����X�N���v�g�i���{��p�j
# 07/07/23

# �n�b�V������ۑ�����f�B���N�g��
#hashdir=./hash_jp0723_1000/
#hashdir=./hash_jp0723_1000/
# ��������̃f�B���N�g���i�ȉ���90/*.jpg���Ƒ����j
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
