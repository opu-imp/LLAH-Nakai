#include "ncr.h"

int CalcnCr( int n, int r )
{
	int i, num, den;
	for ( i = 0, num = 1, den = 1; i < r; i++ ) {
		num *= n - i;
		den *= i + 1;
	}
	return (int)(num / den);
}

