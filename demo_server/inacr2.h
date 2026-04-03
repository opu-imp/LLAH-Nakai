//#define	kMaxPointNum	(2500)	/* �_�̍ő吔 */
#define	kMaxPointNum	(5000)	/* �_�̍ő吔 */
#define	kDiscRes	(1000)	/* ���U���t�@�C���̉𑜓x */

//int LoadPointFile( char *fname, cv::Point **ps0, cv::Size *size );
//void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 );
//void NearestPoint( int n, cv::Point *ps, int num, int nears[] );
//void CalcOrderCWN( int pt, cv::Point *ps, int *nears[], int idx[], int num );
void Hist2Disc( strHist *hist, strDisc *disc, int disc_num );
int SaveDisc( char *fname, strDisc *disc );
int MakeDiscFile( int doc_num, cv::Point **pss, int *nums, strDisc *disc, char *disc_file_name );
