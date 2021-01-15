#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kDiscRes	(1000)	/* 離散化ファイルの解像度 */

int LoadPointFile( char *fname, CvPoint **ps0, CvSize *size );
//void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
//void NearestPoint( int n, CvPoint *ps, int num, int nears[] );
void CalcCRAndAddHist( CvPoint *ps, int num, int *nears[], strHist *hist );
void CalcOrderCWN( int pt, CvPoint *ps, int *nears[], int idx[], int num );
void Hist2Disc( strHist *hist, strDisc *disc, int disc_num );
void SaveDisc( char *fname, strDisc *disc );
