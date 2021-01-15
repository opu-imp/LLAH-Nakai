#define	MERGE_HOR	(0)
#define	MERGE_VER	(1)

int SetPix( IplImage *img, int x, int y, char *val );
int GetPix( IplImage *img, int x, int y, char *val );
int MergeImage( IplImage *img1, IplImage *img2, int mode, IplImage *merge );
void DrawPolygon( IplImage *img, CvPoint *ps, int pnum, CvScalar color, int thickness, int line_type, int shift );
void PutImage( IplImage *src, IplImage *dst, CvPoint origin );
void CopyImageData( IplImage *src, IplImage *dst );
