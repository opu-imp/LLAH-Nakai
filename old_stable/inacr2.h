#define	kMaxPointNum	(2500)	/* �_�̍ő吔 */
#define	kDiscRes	(1000)	/* ���U���t�@�C���̉𑜓x */

//int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size );
//void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
//void NearestPoint( int n, CvPoint *ps, int num, int nears[] );
//void CalcOrderCWN( int pt, CvPoint *ps, int *nears[], int idx[], int num );
void Hist2Disc( strHist *hist, strDisc *disc, int disc_num );
int SaveDisc( char *fname, strDisc *disc );
int MakeDiscFile( int doc_num, CvPoint **pss, int *nums, strDisc *disc );
