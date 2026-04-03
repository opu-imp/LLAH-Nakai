#include "proj4p.h"

#define	kNearParamThr	(1000.0L)
#define	kErrorThrMeanParam	(0.00001)	/* MeanParam�ɂ�����v�Z�ł��؂�̂��߂̃G���[��臒l */
#define	kNumVectorMeanParam	(30)	/* MeanParam�ɂ����鎟��̌v�Z�ɗp����x�N�g���̐� */

// RANSAC�ł̃p�����[�^
#define	kRANSACTry	(100)	/* �p�����[�^�̌v�Z�� */
#define	kRANSACThr	(10.0)	/* �����i�̂Q��j��臒l */
#define	kRANSACMinScore	(4)	/* �ŏ��X�R�A */
#define	kRANSACRefineNum	(100)	/* ��萸�x�̍����p�����[�^�����߂�ۂ̐� */

// CalcProjParamTop�ł̃��[�h
#define	PROJ_NORMAL	(1)
#define	PROJ_REVERSE	(2)
// CalcProjParamTop�ł̐���@
#define	PARAM_IWAMURA	(1)
#define	PARAM_RANSAC	(2)

void RPTMain( char *fname1, char *fname2 );
void RecoverPTTest( cv::Mat img1, cv::Mat img2, cv::Point *p1, cv::Point *p2 );
void RecovPTSub( cv::Point *ps, cv::Point *corps, int cor[][2], int cor_num, cv::Size size, cv::Mat img );
double GetPPVar( cv::Point *ps, cv::Point *corps, int cor[][2], int cor_num );
void CalcProjParamTop( cv::Point *ps, cv::Point *corps, int cor[][2], int cor_num, strProjParam *param, int mode, int method );
int GetAppropriateParamRANSAC( cv::Point *ps, cv::Point *corps, int cor[][2], int cor_num, strProjParam *param, int mode );
int EvaluateParamRANSAC( cv::Point *ps, cv::Point *corps, int cor[][2], int cor_num, strProjParam *param, int mode );
void GetRefinedParamRANSAC( cv::Point *ps, cv::Point *corps, int cor[][2], int cor_num, strProjParam *param_src, strProjParam *param_dst, int mode );
