#include "proj4p.h"

#define	kNearParamThr	(1000.0L)
#define	kErrorThrMeanParam	(0.00001)	/* MeanParamにおける計算打ち切りのためのエラーの閾値 */
#define	kNumVectorMeanParam	(30)	/* MeanParamにおける次回の計算に用いるベクトルの数 */

// RANSACでのパラメータ
#define	kRANSACTry	(100)	/* パラメータの計算回数 */
#define	kRANSACThr	(10.0)	/* 距離（の２乗）の閾値 */
#define	kRANSACMinScore	(4)	/* 最小スコア */
#define	kRANSACRefineNum	(100)	/* より精度の高いパラメータを求める際の数 */

// CalcProjParamTopでのモード
#define	PROJ_NORMAL	(1)
#define	PROJ_REVERSE	(2)
// CalcProjParamTopでの推定法
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
