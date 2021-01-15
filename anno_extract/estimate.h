#define	kTryParamRand	(1000)	/* ランダムに射影変換のパラメータの推定を行う回数 */

// RANSACでのパラメータ
#define	kRANSACTry	(100)	/* パラメータの計算回数 */
#define	kRANSACThr	(100.0)	/* 距離（の２乗）の閾値 */
#define	kRANSACMinScore	(4)	/* 最小スコア */
#define	kRANSACRefineNum	(100)	/* より精度の高いパラメータを求める際の数 */

typedef int keytype;

void EstimateParam( CvPoint **clpsa, CvPoint **clpso, int cor[][4], int cor_num, double *param, int type );
int EstimateParamRANSAC( CvPoint **clpsa, CvPoint **clpso, int cor[][4], int cor_num, double *param, int type );
void GetNumRand( int num, int max, int *arr );
void GetAppropriateParam( double **paramarr, double *param, int type );
void MeanParam( double **data, double *mean, int n, int trpnum, double err0, int threshold);
void simplesort(double *d, keytype *a, int first, int last);
int EvaluateParamRANSAC( CvPoint **clpsa, CvPoint **clpso, int cor[][4], int cor_num, double *param, int type );
