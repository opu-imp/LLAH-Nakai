//#include "hash.h"

typedef struct _strVoteList {
	int doc;
	int point;
	int pat;
	int vote;
	struct _strVoteList *next;
} strVoteList;

typedef struct _strRandPs {
	int pindex;
	int rand_value;
} strRandPs;

#define	kDistNeighbor	(500)
#define	kNearVector	(500)
#define	kEnoughNum	(3)
#define	kVoteThr	(10)
#define	kPNum	(10)
// �ł��؂�֌W
#define	kTermPNum	(50)
#define	kTermVNum	(10)

#define	kDefaultTermVNum	(10)

void CalcCRDiscriptorNN5(cv::Point *ps, int num, int *nears[]);
void CalcCRDiscriptorNN5Test(cv::Point *ps, int num, int *nears[]);
void CalcInterPoint( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point *p1234 );
double CalcCR5( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
double CalcJI5( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
int CalcPerm(int n, int r, int i, int rslt[]);
void CalcOrderCW( int pt, cv::Point *ps, int *nears[], int idx[] );
int RetrieveNN5(cv::Point *ps, int num, int *nears[],  cv::Size size, int *score, strHList **hash, strHList2 **hash2, HENTRY *hash3);
int RetrieveNN5Cor(cv::Point *ps, int num, int *nears[], cv::Size size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 );
int RetrieveNN52( cv::Point *ps, double *areas, int num, int *nears[], cv::Size size, int *score, strDisc *disc, int *reg_nums, int *ret_time, strHList **hash, strHList2 **hash2, HENTRY *hash3);
int RetrieveNN5Cor2(cv::Point *ps, double *areas, int num, int *nears[], cv::Size size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, int *reg_nums, strHList **hash, strHList2 **hash2, HENTRY *hash3 );
int RetrieveAndRecover(cv::Point *ps, int num, int *nears[], cv::Size size, char *img_fname, int mode, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void CalcScore3(cv::Point *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void CalcScore4(cv::Point *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void CalcOrderCWFromNearest( int pt, cv::Point *ps, int *nears[], int idx[] );
int HaveEnoughSimilarVector( cv::Point start, cv::Point end, int idx );
//void DrawCor( void );
void DrawCor( cv::Point *ps, int num, cv::Size img_size, int res, cv::Point *corps, int cornum, cv::Size corsize, int pcor[][2], int pcornum );
//void ConstructHash(int mode);
//int ConstructHash2( int doc_num, cv::Point **pss, double **areass, int *nums, strDisc *disc );
int ConstructHash2( int doc_num, cv::Point **pss, double **areass, int *nums, strDisc *disc, strHList ***ptr_hash, strHList2 ***ptr_hash2, HENTRY **ptr_hash3 );
//int ConstructHashSub( char *fname, int n, int mode, strDisc *disc );
void ConstructHashSubSub(cv::Point *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void ConstructHashSubSub2(cv::Point *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void DrawNeighbor( cv::Size size, cv::Point *ps, int num, int *nears[] );
void CalcOrderCWN( int pt, cv::Point *ps, int *nears[], int idx[], int num );
void VoteByVL( int *score );
void ClearVL( void );
void ClearVLM( void );
void SavePointFile( char *fname, cv::Point *ps, int num, cv::Size *size, double *areas );
void DrawP2P( cv::Point *ps, int num, int cor_doc, cv::Size size );
void RecovPT( cv::Point *ps, int num, int cor_doc, cv::Size size, cv::Mat img );
void RecovPTFromTemp( void );
//void ConstructHashAdd(int mode);
void CheckByPPVar( cv::Point *ps, int num, int *score );
int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size );
int IsDat( char *str );
int IsTxt( char *str );
void GenerateCombination( int inv_type, int g1, int g2, int g3, void (*setcom1)(int, int, int), void (*setcom2)(int, int, int));
void SaveAreaFile( char *fname, double *areas, int num );
void LoadPointFileAll( cv::Point ***psall0, cv::Size **sizeall0, int **numall0 );
void SetCom1( int i, int j, int n );
void SetCom2( int i, int j, int n );
#ifndef WIN
void CreatePointFile2( void );
#endif
void DrawPoints( cv::Size img_size, cv::Point *ps, int num );
int CreatePointFile3( cv::Point ***p_reg_pss, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors );
void CalcCRAndAddHist( cv::Point *ps, int num, int *nears[], strHist *hist );
void ClearFlag1( void );
void ClearFlag2( void );
void ClearCorres( void );
int LoadPointFile2( char *fname, cv::Point ***p_reg_pss, double ***p_reg_areass, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors );
int FindStartPoint( char *inv_array );
int CreatePointFile4( cv::Point ***p_reg_pss, double ***p_reg_areass, cv::Size **p_reg_sizes, int **p_reg_nums, char ***p_dbcors );
void CalcHindexArea( int *idxcom1, double *areas, char *hindex_area );
int VoteByHashEntry( HENTRY *phe, int *score, int p );
