IplImage *GetConnectedImage( char *fname, int mode );
int AutoConnectTest( char *fname, CvSize *size );
IplImage *GetConnectedImageCam( IplImage *src, IplImage *inv, int cam_gauss_mask_size );
IplImage *GetConnectedImageCamJp( IplImage *src );
IplImage *GetConnectedImageReg( IplImage *img );
IplImage *GetConnectedImageRegJp( IplImage *src );
IplImage *GetConnectedImageCamJp2( IplImage *src );
