#!/bin/sh
# �o�^�X�N���v�g(Open Colledge�p)
# 07/08/10

# �o�^����摜�̃f�B���N�g��
#dbdir=/home/nakai/didb/jp/jpg/10000/
#dbdir=/home/nakai/didb/jp/1000/
#dbdir=/home/nakai/didb/test100_2/
dbdir=/home/nakai/didb/opencollege/
# �n�b�V������ۑ�����f�B���N�g��
#hashdir=./hash_jp0723_10000/
#hashdir=./hash_jp0723_1000/
hashdir=/home/nakai/dirs/oc07/

# �摜��p���郂�[�h
./dcams -e -c $dbdir bmp -h $hashdir
