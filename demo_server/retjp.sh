#!/bin/sh

# �o�^�摜���i�����̒l�ɃZ�b�g �����Ȃ��△���Ȓl�Ȃ�100�j
if [ $# -ge 1 ];
then
	case $1 in
		100 | 1000 | 10000)	num=$1;;
		20000)	num=20596;;
		*)	num=100;;
	esac
else
	num=100
fi
#num=10000
# �e��ݒ�t�@�C���̊i�[�f�B���N�g��
ddir="/home/nakai/dirs/jp200/"
# ���ʏo�̓f�B���N�g��
odir="out/"

./dcams -S -I $ddir
