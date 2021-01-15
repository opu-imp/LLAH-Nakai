#include <stdlib.h>
#include "score.h"

int score_compare( const strScore *a, const strScore *b )
{
	if ( a->vote > b->vote )	return -1;
	else if ( a->vote < b->vote )	return 1;
	else	return 0;
}

void SetAndSortScore( strScore *score, int *dat, int n )
// strScore構造体に入れてソートする
{
	int i;
	
	for ( i = 0; i < n; i++ ) {
		score[i].n = i;
		score[i].vote = dat[i];
	}
	// ソート
	qsort(score, n, sizeof(strScore), (int(*)(const void*, const void*))score_compare);
}
