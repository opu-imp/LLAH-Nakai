/***********************************************************
    qsort1.c -- クイックソート
***********************************************************/
typedef int keytype;

void quicksort(double *d, keytype *a, int first, int last)
{
    int i, j;
    keytype x, t;

    x = (int)(d[a[(first + last) / 2]]);
    i = first;  j = last;
    for ( ; ; ) {
        while (d[a[i]] < x) i++;
        while (x < d[a[j]]) j--;
        if (i >= j) break;
        t = a[i];  a[i] = a[j];  a[j] = t;
        i++;  j--;
    }
    if (first  < i - 1) quicksort(d, a, first , i - 1);
    if (j + 1 < last) quicksort(d, a, j + 1, last);
}

void simplesort(double *d, keytype *a, int first, int last)
{
	int i, j, max_n, tmp_n;
	double max, tmp;

	for ( i = last; i > first; i-- ) {
		max = 0.0L;
		max_n = first;
		for ( j = first; j <= i; j++ ) {
			if ( d[j] >= max ) {
				max = d[j];
				max_n = j;
			}
		}
		tmp = d[max_n];
		tmp_n = a[max_n];
		d[max_n] = d[i];
		a[max_n] = a[i];
		d[i] = tmp;
		a[i] = tmp_n;
	}
}
