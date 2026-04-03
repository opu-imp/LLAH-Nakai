#if 0 // TODO: This file uses internal OpenCV APIs (GrFmtReader, grfmts.h, CV_CALL)
// that were removed in OpenCV 4. Needs complete rewrite using cv::imread.
//
//  Loading and saving IPL images.
//

// #include "_highgui.h" // removed for OpenCV 4
#include "grfmts.h"

/****************************************************************************************\
*                         HighGUI loading & saving function implementation               *
\****************************************************************************************/

GrFmtReader**
cvLoadImage_prepare_nakai( const char** filename_list, int num_fname )
{
  GrFmtReader** reader =
    (GrFmtReader**)mallock(sizeof(GrFmtReader*)*num_fname);
  
  for (int i=0; i<num_fname; i++) {
    /* codes from icvLoadImage */
    if( !filename || strlen(filename_list[i]) == 0 )
      CV_ERROR( CV_StsNullPtr, "null filename" );

    reader[i] = g_Filters.FindReader( filename );
    if( !reader )
      EXIT;
  }

  return reader;
}

/*
void
cvLoadImage_finish_nakai( GrFmtReader** reader )
{
  delete reader;
}
*/

static void*
/*icvLoadImage_nakai( const char* filename, int flags, bool load_as_matrix )*/
icvLoadImage_nakai( GrFmtReader* reader, int flags, bool load_as_matrix )
{
  /*    GrFmtReader* reader = 0; */
    cv::Mat  image = 0;
    cv::Mat hdr, *matrix = 0;
    int depth = 8;

    CV_FUNCNAME( "cvLoadImage_nakai" );

    __BEGIN__;

    cv::Size size;
    int iscolor;
    int cn;

    /*
    if( !filename || strlen(filename) == 0 )
        CV_ERROR( CV_StsNullPtr, "null filename" );

    reader = g_Filters.FindReader( filename );
    if( !reader )
        EXIT;
    */

    if( !reader->ReadHeader() )
        EXIT;

    size.width = reader->GetWidth();
    size.height = reader->GetHeight();

    if( flags == -1 )
        iscolor = reader->IsColor();
    else
    {
        if( (flags & CV_LOAD_IMAGE_COLOR) != 0 ||
           ((flags & CV_LOAD_IMAGE_ANYCOLOR) != 0 && reader->IsColor()) )
            iscolor = 1;
        else
            iscolor = 0;

        if( (flags & CV_LOAD_IMAGE_ANYDEPTH) != 0 )
        {
            reader->UseNativeDepth(true);
            depth = reader->GetDepth();
        }
    }

    cn = iscolor ? 3 : 1;

    if( load_as_matrix )
    {
        int type;
        if(reader->IsFloat() && depth != 8)
            type = CV_32F;
        else
            type = ( depth <= 8 ) ? CV_8U : ( depth <= 16 ) ? CV_16U : CV_32S;
        CV_CALL( matrix = cvCreateMat( size.height, size.width, CV_MAKETYPE(type, cn) ));
    }
    else
    {
        int type;
        if(reader->IsFloat() && depth != 8)
            type = IPL_DEPTH_32F;
        else
            type = ( depth <= 8 ) ? CV_8U : ( depth <= 16 ) ? IPL_DEPTH_16U : IPL_DEPTH_32S;
        CV_CALL( image = cvCreateImage( size, type, cn ));
        matrix = cvGetMat( image, &hdr );
    }

    if( !reader->ReadData( matrix->data.ptr, matrix->step, iscolor ))
    {
        if( load_as_matrix ) // cvReleaseMat removed (cv::Mat auto-manages memory)
        else // cvReleaseImage removed (cv::Mat auto-manages memory)
        EXIT;
    }

    __END__;

    delete reader;

    if( cvGetErrStatus() < 0 )
    {
        if( load_as_matrix ) // cvReleaseMat removed (cv::Mat auto-manages memory)
        else // cvReleaseImage removed (cv::Mat auto-manages memory)
    }

    return load_as_matrix ? (void*)matrix : (void*)image;
}


CV_IMPL cv::Mat 
cvLoadImage_nakai( GrFmtReader* reader, int iscolor )
{
    return (cv::Mat )icvLoadImage_nakai( reader, iscolor, false );
}

/* End of file. */

#endif // if 0
