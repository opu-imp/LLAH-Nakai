#!/bin/sh
# �o�^�X�N���v�g�i���{��p�j
# 07/07/23

# �o�^����摜�̃f�B���N�g��
#dbdir=/home/nakai/didb/jp/jpg/10000/
#dbdir=/home/nakai/didb/jp/1000/
#dbdir=/home/nakai/didb/test100_2/
dbdir=/home/nakai/didb/jp/miru06_1000/
# �n�b�V������ۑ�����f�B���N�g��
#hashdir=./hash_jp0723_10000/
#hashdir=./hash_jp0723_1000/
hashdir=/home/nakai/dirs/jp/miru06_1000/

# �摜��p���郂�[�h
#./dcams -J -e -c $dbdir jpg -h $hashdir >> out0724_2.txt
#./dcams -e -c $dbdir bmp -h $hashdir >> out0723.txt
./dcams -J -e -u -c $dbdir jpg -h $hashdir
# ������point.dat��p���郂�[�h
#./dcams -J -u -e -c /home/nakai/didb/jp/jpg/100/ jpg -h ./hash_jp0723_100/
#./dcams -J -e -c /home/nakai/didb/jp/jpg/100/ jpg -h ./hash_jp0723_100_2/
#./dcams -J -u -e -c /home/nakai/didb/jp/jpg/1000/ jpg -h ./hash_jp0723_1000/
#./dcams -J -e -c /home/nakai/didb/jp/jpg/1000/ jpg -h ./hash_jp0723_1000_2/
##./dcams -J -e -u -x -h $hashdir
##./dcams -u -x -h $hashdir >> out0723.txt
#
#./dcams -J -e -h ./hash_jp0723_100_2/ "$quedir"*.JPG >> out0724_3.txt
#./dcams -J -e -h ./hash_jp0723_100/ "$quedir"*.JPG >> out0724_3.txt
#./dcams -J -e -h ./hash_jp0723_1000_2/ "$quedir"*.JPG >> out0724_3.txt
#./dcams -J -e -h ./hash_jp0723_1000/ "$quedir"*.JPG >> out0724_3.txt

#./dcams -J -e -x -h ./hash_jp0723_10000_2/ >> out0724_4.txt
#./dcams -J -e -h ./hash_jp0723_10000_2/ "$quedir"*.JPG >> out0724_4.txt
#./dcams -J -e -u -x -h ./hash_jp0723_10000/ >> out0724_4.txt
#./dcams -J -e -h ./hash_jp0723_10000/ "$quedir"*.JPG >> out0724_4.txt

#./dcams -J -e -x -h ./hash_jp0723_100_2/
#./dcams -J -e -u -x -h ./hash_jp0723_100/
#./dcams -J -e -x -h ./hash_jp0723_1000_2/
#./dcams -J -e -u -x -h ./hash_jp0723_1000/
