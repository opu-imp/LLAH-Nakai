#include <opencv2/opencv.hpp>

void ClusterTest( void )
// KMeans2�̃e�X�g
{
#if 0
	int a=0;

    #define MAX_CLUSTERS 5
    cv::Scalar color_tab[MAX_CLUSTERS];
    cv::Mat  img = cv::Mat::zeros(cv::Size( 500, 500 ), CV_8UC3);
    cv::RNG rng = cv::RNG(0xffffffff);
    
    color_tab[0] = cv::Scalar(0,0,255);
    color_tab[1] = cv::Scalar(0,255,0);
    color_tab[2] = cv::Scalar(255,100,100);
    color_tab[3] = cv::Scalar(255,0,255);
    color_tab[4] = cv::Scalar(0,255,255);

    cv::namedWindow( "clusters", 1 );

    for(;;)
    {
        int k, cluster_count = cv_randint_NEEDS_FIX(&rng)%MAX_CLUSTERS + 1;
        int i, sample_count = cv_randint_NEEDS_FIX(&rng)%1000 + 1;
        cv::Mat points = cv::Mat(sample_count, 1, CV_32FC2 , cv::Scalar(0));
        cv::Mat clusters = cv::Mat(sample_count, 1, CV_32SC1 , cv::Scalar(0));

        /* generate random sample from multigaussian distribution */
        for( k = 0; k < cluster_count; k++ )
        {
            cv::Point center;
            cv::Mat point_chunk;
            center.x = cv_randint_NEEDS_FIX(&rng)%img.cols;
            center.y = cv_randint_NEEDS_FIX(&rng)%img.rows;
            cv_getrows_NEEDS_FIX( points, &point_chunk, k*sample_count/cluster_count,
                       k == cluster_count - 1 ? sample_count : (k+1)*sample_count/cluster_count );
            cv_randarr_NEEDS_FIX( &rng, &point_chunk, cv::RNG::NORMAL,
                       cvScalar(center.x,center.y,0,0),
                       cvScalar(img.cols/6, img.rows/6,0,0) );
        }

        /* shuffle samples */
        for( i = 0; i < sample_count/2; i++ )
        {
            cv::Point2f* pt1 = (cv::Point2f*)points->data.fl + cv_randint_NEEDS_FIX(&rng)%sample_count;
            cv::Point2f* pt2 = (cv::Point2f*)points->data.fl + cv_randint_NEEDS_FIX(&rng)%sample_count;
            cv::Point2f temp;
            std::swap( *pt1, *pt2, temp );
        }

        cv::kmeans( points, cluster_count, clusters,
                   cv::TermCriteria( cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 10, 1.0 ));

        img.setTo(cv::Scalar::all(0));

        for( i = 0; i < sample_count; i++ )
        {
            cv::Point2f pt = ((cv::Point2f*)points->data.fl)[i];
            int cluster_idx = clusters->data.i[i];
            cv::circle( img, (cv::Point)(pt), 2, color_tab[cluster_idx], cv::FILLED );
        }


        cv::imshow( "clusters", img );

        int key = cv::waitKey(0);
        if( key == 27 ) // 'ESC'
            break;
    }
#endif
	return;
}
