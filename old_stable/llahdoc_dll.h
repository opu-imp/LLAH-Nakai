#ifndef	__LLAHDOC_DLL_H__
#define	__LLAHDOC_DLL_H__

#ifdef	LLAHDOC_EXPORTS
#ifdef __cplusplus
#define	_EXPORT extern "C" __declspec(dllexport)
#else
#define _EXPORT __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define	_EXPORT extern "C" __declspec(dllimport)
#else
#define _EXPORT __declspec(dllimport)
#endif
#endif	/* LLAHDOC_EXPORTS */

#include "hash.h"
#include "disc.h"
#include "cxtypes.h"

typedef struct _strLlahDocDb {
	strDisc disc;
	strHList **hash;
	strHList2 **hash2;
	HENTRY *hash3;
	CvPoint **reg_pss;
	double **reg_areass;
	CvSize *reg_sizes;
	int *reg_nums;
	char **dbcors;
} strLlahDocDb;

// Constant numbers for LlahDocRetrieveIplImage()
#define	kTopThr	0	/* Threshold for reject */
#define	kMinPoints	(20)	/* Minnimum points to retrieve */

_EXPORT	void Version( void );
_EXPORT int LlahDocConstructDb( char inv, int n, int m, int d, const char *reg_dir, const char *reg_suffix, const char *db_dir );
_EXPORT strLlahDocDb *LlahDocLoadDb( const char *db_dir );
_EXPORT void LlahDocReleaseDb( strLlahDocDb *db );
_EXPORT int LlahDocRetrieveImage( const char *img_fname, strLlahDocDb *db, char *result, int result_len );
_EXPORT int LlahDocRetrieveIplImage( IplImage *img, strLlahDocDb *db, char *result, int result_len );

#endif	/* __LLAHDOC_DLL_H__ */
