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
// ë≈ÇøêÿÇËä÷åW
#define	kTermPNum	(50)
#define	kTermVNum	(10)

#define	kDefaultTermVNum	(10)

void CalcCRDiscriptorNN5(CvPoint *ps, int num, int *nears[]);
void CalcCRDiscriptorNN5Test(CvPoint *ps, int num, int *nears[]);
void CalcInterPoint( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint *p1234 );
double CalcCR5( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
double CalcJI5( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
int CalcPerm(int n, int r, int i, int rslt[]);
void CalcOrderCW( int pt, CvPoint *ps, int *nears[], int idx[] );
int RetrieveNN5(CvPoint *ps, int num, int *nears[],  CvSize size, int *score, strHList **hash, strHList2 **hash2, HENTRY *hash3);
int RetrieveNN5Cor(CvPoint *ps, int num, int *nears[], CvSize size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 );
int RetrieveNN52( CvPoint *ps, double *areas, int num, int *nears[], CvSize size, int *score, strDisc *disc, int *reg_nums, int *ret_time, strHList **hash, strHList2 **hash2, HENTRY *hash3);
int RetrieveNN5Cor2(CvPoint *ps, double *areas, int num, int *nears[], CvSize size, int *score, int pcor[][2], int *pcornum0, strDisc *disc, int *reg_nums, strHList **hash, strHList2 **hash2, HENTRY *hash3 );
int RetrieveAndRecover(CvPoint *ps, int num, int *nears[], CvSize size, char *img_fname, int mode, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void CalcScore3(CvPoint *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void CalcScore4(CvPoint *ps, double *areas, int num, int *nears[], int *score, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void CalcOrderCWFromNearest( int pt, CvPoint *ps, int *nears[], int idx[] );
int HaveEnoughSimilarVector( CvPoint start, CvPoint end, int idx );
//void DrawCor( void );
void DrawCor( CvPoint *ps, int num, CvSize img_size, int res, CvPoint *corps, int cornum, CvSize corsize, int pcor[][2], int pcornum );
//void ConstructHash(int mode);
//int ConstructHash2( int doc_num, CvPoint **pss, double **areass, int *nums, strDisc *disc );
int ConstructHash2( int doc_num, CvPoint **pss, double **areass, int *nums, strDisc *disc, strHList ***ptr_hash, strHList2 ***ptr_hash2, HENTRY **ptr_hash3 );
//int ConstructHashSub( char *fname, int n, int mode, strDisc *disc );
void ConstructHashSubSub(CvPoint *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void ConstructHashSubSub2(CvPoint *ps, double *areas, int num, int *nears[], int n, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3);
void DrawNeighbor( CvSize size, CvPoint *ps, int num, int *nears[] );
void CalcOrderCWN( int pt, CvPoint *ps, int *nears[], int idx[], int num );
void VoteByVL( int *score );
void ClearVL( void );
void ClearVLM( void );
void SavePointFile( char *fname, CvPoint *ps, int num, CvSize *size, double *areas );
void DrawP2P( CvPoint *ps, int num, int cor_doc, CvSize size );
void RecovPT( CvPoint *ps, int num, int cor_doc, CvSize size, IplImage *img );
void RecovPTFromTemp( void );
//void ConstructHashAdd(int mode);
void CheckByPPVar( CvPoint *ps, int num, int *score );
int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size );
int IsDat( char *str );
int IsTxt( char *str );
void GenerateCombination( int inv_type, int g1, int g2, int g3, void (*setcom1)(int, int, int), void (*setcom2)(int, int, int));
void SaveAreaFile( char *fname, double *areas, int num );
void LoadPointFileAll( CvPoint ***psall0, CvSize **sizeall0, int **numall0 );
void SetCom1( int i, int j, int n );
void SetCom2( int i, int j, int n );
#ifndef WIN32
void CreatePointFile2( void );
#endif
void DrawPoints( CvSize img_size, CvPoint *ps, int num );
int CreatePointFile3( CvPoint ***p_reg_pss, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors );
void CalcCRAndAddHist( CvPoint *ps, int num, int *nears[], strHist *hist );
void ClearFlag1( void );
void ClearFlag2( void );
void ClearCorres( void );
int LoadPointFile2( char *fname, CvPoint ***p_reg_pss, double ***p_reg_areass, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors );
int FindStartPoint( char *inv_array );
int CreatePointFile4( CvPoint ***p_reg_pss, double ***p_reg_areass, CvSize **p_reg_sizes, int **p_reg_nums, char ***p_dbcors );
void CalcHindexArea( int *idxcom1, double *areas, char *hindex_area );
int VoteByHashEntry( HENTRY *phe, int *score, int p );
void DrawPointAttribute(  CvPoint *ps, int num, CvSize img_size, char *pattr );
int RetrieveNN5CorAll(CvPoint *ps, double *areas, int num, int *nears[], CvSize size, int *score, int pcors[][kMaxPointNum][2], int *pcornums, strDisc *disc, int *reg_nums, strHList **hash, strHList2 **hash2, HENTRY *hash3 );
