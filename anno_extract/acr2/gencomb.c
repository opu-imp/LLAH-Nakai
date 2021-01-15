/*		comb.c		組合せ(nＣk)の生成	generation of combination		*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define		set			unsigned int

set K, N, x, overx;

int subcomb(int p[]);	/* gencomb1() のスレーブルーチン */
set nextset(void);		/* gencomb1() のスレーブルーチン */

void gencomb(int n, int k, void (*setnum)(int, int, int))
{
	int i, j, *p, *q, count;
	set s;

	p = (int *)malloc(n*sizeof(int));
	count = 0;
	N = n;
	K = k;
	x = (1 << K) - 1L;
	overx = ~((1 << N) - 1L);
	for(j = 1, q = p, s = x; j <= N; j++)
	{
		if(s & 1)	*q++ = j;
		s >>= 1;
	}
	for(i = 0, q = p; i < k; i++)	setnum(count, i, (*q++)-1);
	count++;
	while(subcomb(p))  {
		for(i = 0, q = p; i < k; i++) 	setnum(count, i, (*q++)-1);
		count++;
	}
	free(p);
}

int subcomb(int p[])
{
	int j, *q;
	set s;

	x = nextset();
	if(x & overx)	return 0;
	for(j = 1, q = p, s = x; j <= N; j++)
	{
		if(s & 1)	*q++ = j;
		s >>= 1;
	}
	return 1;
}

set nextset(void)
{
	set smallest, ripple, new_smallest, ones;

	smallest = x & -x;
	ripple = x + smallest;
	new_smallest = ripple & -ripple;
	ones = ((new_smallest / smallest) >> 1) - 1;
	return ripple | ones;
}
