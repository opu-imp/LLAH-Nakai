#!/bin/sh

# �o�^�摜�̌����p�X
rpath="/home/nakai/didb/jp/test200/*.bmp"
# �o�^����摜�̓_�t�@�C���̃f�B���N�g��
pdir="/home/nakai/dirs/point/jp200/"
# �_�t�@�C���̑Ή��t�@�C��
dbcor="/home/nakai/dirs/dbcor/jp200/dbcor.dat"
# �����_���t�@�C��
pn="/home/nakai/dirs/pnum/jp200/pnum.txt"
# �J�n�i���o�[�i�f�t�H���g��0�j
start=0

./dcams -JG "$rpath" $pdir $dbcor $pn $start
