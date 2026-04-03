cv::Mat GetConnectedImage( char *fname, int mode );
int AutoConnectTest( char *fname, cv::Size *size );
cv::Mat GetConnectedImageCam( cv::Mat src, cv::Mat inv, int cam_gauss_mask_size );
cv::Mat GetConnectedImageCamJp( cv::Mat src );
cv::Mat GetConnectedImageReg( cv::Mat img );
cv::Mat GetConnectedImageRegJp( cv::Mat src );
cv::Mat GetConnectedImageCamJp2( cv::Mat src );
