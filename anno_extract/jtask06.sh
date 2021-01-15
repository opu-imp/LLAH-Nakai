#!/bin/sh
# origÇÃÇ†ÇÈÇ‡ÇÃÇæÇØÇ≈é¿çs

# Dir name setting

origdir=/home/nakai/jutaku/600dpi/orig/
annodir=/home/nakai/jutaku/600dpi/
outdir=out070218/
annex=./annex

n=5
m=5
disc=50
cluster=5

annofname=$annodir"0483.jpg"
origfname=$annodir"0480.jpg"

$annex "-n"$n "-m"$m "-d"$disc "-c"$cluster -E 10000000000 -o $outdir $origfname $annofname
