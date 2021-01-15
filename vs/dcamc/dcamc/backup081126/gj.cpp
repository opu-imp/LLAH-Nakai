#include <stdio.h>
#include "gj.h"

void gj( double a[kLinearVarNum][kLinearVarNum+1] )
{
    int i,j,k,n1;
    double w;

    n1 = kLinearVarNum + 1;
    for ( k = 0; k < kLinearVarNum; k++ ) {
        w = a[k][k];
		if ( w >= 0.0 && w < 0.000001 )	w = 0.000001;	// w == 0.0‚Ì‘Îô
		if ( w <= 0.0 && w > -0.000001 )	w = -0.000001;
//		printf("w = %f\n", w);
        for ( j = k; j < n1; j++ )	a[k][j] = a[k][j] / w;
        for ( i = 0; i < kLinearVarNum; i++) {
            if ( i != k ) {
                w = a[i][k];
                for ( j = k; j < n1; j++ )	a[i][j] = a[i][j] - a[k][j] * w;
			}
		}
	}
}
