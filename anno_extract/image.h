#define	cWhite	cv::Scalar(255,255,255)
#define	cBlack	cv::Scalar(0,0,0)
#define	cRed	cv::Scalar(0,0,255)
#define	cGreen	cv::Scalar(0,255,0)
#define	cBlue	cv::Scalar(255,0,0)
#define	cRandom	cv::Scalar( rand()%256, rand()%256, rand()%256 )

#define	kBinMaxVal	(256)	/* 二値化の際の画素値の最大値 */
#define	kDrawContourMaxLevel	(-1)	/* 連結成分描画時のパラメータ */
#define	kDrawContourLineType	(8)	/* 同上 */
#define	kMaxColorDist	(256*256*256) /* 8ビット3チャネルカラーの最大距離 */

#ifndef FIRST_TOP_LEVEL_CONTOUR_DEFINED
#define FIRST_TOP_LEVEL_CONTOUR_DEFINED
// Index of the first top-level contour (parent == -1). Iterating
//   for ( ci = FirstTopLevelContour( h ); ci >= 0; ci = h[ci][0] )
// visits the same contours as the OpenCV 1.x/2.x  first_contour->h_next  chain.
inline int FirstTopLevelContour( const std::vector<cv::Vec4i> &hierarchy )
{
	for ( size_t k = 0; k < hierarchy.size(); k++ )	if ( hierarchy[k][3] < 0 )	return (int)k;
	return -1;
}
#endif
