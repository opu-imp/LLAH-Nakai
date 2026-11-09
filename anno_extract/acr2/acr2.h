#define	kMaxPointNum	(1024)	/* 点の最大数 */
#define	kDiscRes	(1000)	/* 離散化ファイルの解像度 */

int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size );
//void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 );
//void NearestPoint( int n, cv::Point *ps, int num, int nears[] );
void CalcCRAndAddHist( cv::Point *ps, int num, int *nears[], strHist *hist );
void CalcOrderCWN( int pt, cv::Point *ps, int *nears[], int idx[], int num );
void Hist2Disc( strHist *hist, strDisc *disc, int disc_num );
void SaveDisc( char *fname, strDisc *disc );
