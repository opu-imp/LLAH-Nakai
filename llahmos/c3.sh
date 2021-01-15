#!/bin/sh

# 登録する画像のディレクトリ
#dbdir=/home/nakai/didb/test1000/
dbdir=/home/nakai/didb/test100_2/
#dbdir=/home/nakai/didb/jp/100/

#./dcams -x -y -z
#echo "Normal" >> out0510.txt
#./dcams -e -y /home/nakai/image/3deg100/60/*.dat >> out0510.txt

# 画像を用いるモード
#./dcams -c $dbdir bmp -y -z -w 1 -v -u >> out0530.txt
# 既存のpoint.datを用いるモード
#./dcams -x -y -z -w 1 -v -u
#echo "Refined 1" >> out0529_4.txt
#./dcams -e -y -v -u /home/nakai/image/3deg100/jpg/60/*.jpg >> out0530area.txt
#./dcams -e -y -v -u /home/nakai/image/3deg100/jpg/60/cvpr012a023.jpg

# 画像を用いるモード
#./dcams -c $dbdir bmp -h ./hash1000/ -y -z -w 1 -v -u -t
# オリジナルモード
./dcams -c $dbdir bmp -h ./hash0606_100/ -e >> out0606_3.txt
# 既存のpoint.datを用いるモード
#./dcams -x -h ./hash0606_10000/ -e >> out0606_2.txt
#./dcams -j -x -y -z -w 1 -v -u
#>>#./dcams -J -x -h ./hash_jp100_2/ -y -z -w 1 -v -u -t
#>>>./dcams -e -y -z -v -u -t -h ./hash1000/ /home/nakai/image/3deg100/jpg/60/*.jpg >> out0601.txt
#./dcams -e -y -z -v -u -t -h ./hash1000/ /home/nakai/image/3deg100/jpg/60/cvpr013a011.jpg
#./dcams -J -e -y -v -u -t -h ./hash_jp100_2/ /home/nakai/image/3deg100jp/90/jnl4101001-0.jpg

#./dcams -x -y -z -v >> out0529.txt
#echo "Normal" >> out0510.txt
./dcams -e -h ./hash0606_100/ /home/nakai/image/3deg100/jpg/45/*.jpg >> out0606_3.txt

./dcams -x -z -w 1 -v -e -t -h ./hash0606_100/ >> out0606_3.txt

./dcams -e -h ./hash0606_100/ -z -v -t /home/nakai/image/3deg100/jpg/45/*.jpg >> out0606_3.txt
./dcams -x -z -w 1 -v -e -t -h ./hash0606_100/ -y >> out0606_3.txt

./dcams -e -h ./hash0606_100/ -z -v -t -y /home/nakai/image/3deg100/jpg/45/*.jpg >> out0606_3.txt
