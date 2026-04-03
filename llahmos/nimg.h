#define	MERGE_HOR	(0)
#define	MERGE_VER	(1)

int SetPix( cv::Mat img, int x, int y, char *val );
int GetPix( cv::Mat img, int x, int y, char *val );
int SetPixU( cv::Mat img, int x, int y, unsigned char *val );
int GetPixU( cv::Mat img, int x, int y, unsigned char *val );
int MergeImage( cv::Mat img1, cv::Mat img2, int mode, cv::Mat merge );
int MergeImage3( cv::Mat img1, cv::Mat img2, cv::Mat img3, cv::Mat merge );
void DrawPolygon( cv::Mat img, cv::Point *ps, int pnum, cv::Scalar color, int thickness, int line_type, int shift );
void PutImage( cv::Mat src, cv::Mat dst, cv::Point origin );
void CopyImageData( cv::Mat src, cv::Mat dst );
int CropImage( cv::Mat src, cv::Mat dst, cv::Point pos );
