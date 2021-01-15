typedef struct _strDiv {	// 分割領域
	int xindex;	// x軸のインデックス
	int yindex;	// y軸のインデックス
	double dist;	// 注目している特徴点との最小距離
} strDiv;

typedef struct _strPointDist {	// ソートのための特徴点と距離の構造体
	int pindex;	// 点のインデックス
	double	dist;	// 点までの距離
} strPointDist;

void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
void NearestPoint( int n, CvPoint *ps, int num, int nears[] );
void MakeNearsFromCentresDiv( CvPoint *ps, int pnum, CvSize *size, int tx, int ty, int kn, int ***nears0 );
double CalcDivDist( CvPoint pt1, CvSize *size, int tx, int ty, int x, int y, int div_x, int div_y );
void MergePointDist( strPointDist *tmp0, int t0num, strPointDist *tmp1, int t1num, int kn, strPointDist **merge0, int *mnum0 );
