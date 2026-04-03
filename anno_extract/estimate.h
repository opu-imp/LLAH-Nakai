#define	kTryParamRand	(1000)	/* �����_���Ɏˉe�ϊ��̃p�����[�^�̐�����s���� */

// RANSAC�ł̃p�����[�^
#define	kRANSACTry	(100)	/* �p�����[�^�̌v�Z�� */
#define	kRANSACThr	(100.0)	/* �����i�̂Q��j��臒l */
#define	kRANSACMinScore	(4)	/* �ŏ��X�R�A */
#define	kRANSACRefineNum	(100)	/* ��萸�x�̍����p�����[�^�����߂�ۂ̐� */

typedef int keytype;

void EstimateParam( cv::Point **clpsa, cv::Point **clpso, int cor[][4], int cor_num, double *param, int type );
int EstimateParamRANSAC( cv::Point **clpsa, cv::Point **clpso, int cor[][4], int cor_num, double *param, int type );
void GetNumRand( int num, int max, int *arr );
void GetAppropriateParam( double **paramarr, double *param, int type );
void MeanParam( double **data, double *mean, int n, int trpnum, double err0, int threshold);
void simplesort(double *d, keytype *a, int first, int last);
int EvaluateParamRANSAC( cv::Point **clpsa, cv::Point **clpso, int cor[][4], int cor_num, double *param, int type );
