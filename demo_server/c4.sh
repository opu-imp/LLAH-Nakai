#!/bin/sh
# �o�^�X�N���v�g�i���{��p�j
# 07/07/23

# �o�^����摜�̃f�B���N�g��
dbdir=/home/nakai/didb/jp/jpg/10000/
#dbdir=/home/nakai/didb/jp/1000/
#dbdir=/home/nakai/didb/test100_2/
# �n�b�V������ۑ�����f�B���N�g��
hashdir=./hash_jp0723_10000_2/
#hashdir=./hash_jp0723_1000/

# �摜��p���郂�[�h
#./dcams -J -e -c $dbdir jpg -h $hashdir >> out0724_2.txt
#./dcams -e -c $dbdir bmp -h $hashdir >> out0723.txt
# ������point.dat��p���郂�[�h
#./dcams -J -u -e -x -h $hashdir >> out0724.txt
./dcams -J -e -x -h $hashdir >> out0724_2.txt
#./dcams -u -x -h $hashdir >> out0723.txt
