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
# �o�^����摜�̓_�t�@�C���̃f�B���N�g��
pdir="/home/nakai/dirs/point/""$num""/"
# �_�t�@�C���̑Ή��t�@�C��
dbcor="/home/nakai/dirs/dbcor/""$num""/dbcor.dat"
# �����_���t�@�C��
pn="/home/nakai/dirs/pnum/""$num""/pnum.txt"
# �o�^�摜�̌����p�X
rpath="/home/nakai/dirs/point/""$num""/*.dat"
# ���U���֐�����ѕ␳�萔�̍쐬�p�f�[�^�̌����p�X
mpath="/home/nakai/dirs/point/10/*.dat"
# ��������̒T���p�X
# qpath="/home/nakai/image/3deg100/60/*.dat"
# �e��ݒ�t�@�C���̊i�[�f�B���N�g��
ddir="/home/nakai/dirs/0421/""$num""/"
# ���ʏo�̓f�B���N�g��
odir="out/"

inv=af
ch=a
n=7
m=6
disc=28
deg=60

opath="$odir""$deg""/""$inv""$n""$m""_""$disc""_""$num"".txt"
apath="$odir""$deg""/avg_""$inv""$n""$m""_""$num"".txt"
qpath="/home/nakai/image/3deg100/""$deg""/*.dat"

./dcams -S -I $ddir
# > "$opath"
# ../ares/ares "$opath" >> "$apath"
