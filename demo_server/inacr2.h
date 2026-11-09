//#define	kMaxPointNum	(2500)	/* 点の最大数 */
#define	kMaxPointNum	(5000)	/* 点の最大数 */
#define	kDiscRes	(1000)	/* 離散化ファイルの解像度 */

//int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size );
//void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 );
//void NearestPoint( int n, cv::Point *ps, int num, int nears[] );
//void CalcOrderCWN( int pt, cv::Point *ps, int *nears[], int idx[], int num );
void Hist2Disc( strHist *hist, strDisc *disc, int disc_num );
int SaveDisc( char *fname, strDisc *disc );
int MakeDiscFile( int doc_num, cv::Point **pss, int *nums, strDisc *disc, char *disc_file_name );
