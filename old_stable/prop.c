#include "def_general.h"
#include <stdio.h>
#ifdef	WIN32
#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#else
#include <glob.h>
#endif
#include <cv.h>
#include <highgui.h>

#include "disc.h"
#include "dirs.h"
#include "hist.h"
#include "hash.h"
#include "nn5.h"
#include "extern.h"
#include "nears.h"
#include "prop.h"

int errfunc2( const char *epath, int eerrno )
{
	printf("%d : %s\n", eerrno, epath );
	return 1;
}

double CalcProp2( int doc_num, CvPoint **pss, double **areass, int *nums, strDisc *disc, strHList **hash, strHList2 **hash2, HENTRY *hash3 )
// ���萔�𒲂ׂ�
{
	int i, j, max_doc, max_score;
    int **nears = 0;
    int score[kMaxDocNum];
    long xs_sum = 0, xy_sum = 0;

	for ( i = 0; i < doc_num; i++ ) {
//		fprintf( stderr, "CalcProp2: %d\n", i);
		MakeNearsFromCentres( pss[i], nums[i], &nears );
		// �e�[�u���E�X�R�A�̏�����
		ClearFlag2();
		ClearCorres();
		memset( score, 0, sizeof(int) );
		// �������s���C���[�������߂�
		if ( eRotateOnce )	CalcScore4( pss[i], areass[i], nums[i], nears, score, disc, hash, hash2, hash3 );
		else				CalcScore3( pss[i], areass[i], nums[i], nears, score, disc, hash, hash2, hash3 );
		// �ő�X�R�A�i�����j�̕���ID�����߂�
		for ( j = 0, max_doc = -1, max_score = -1; j < eDbDocs; j++ ) {
			if ( score[j] > max_score ) {
				max_doc = j;
				max_score = score[j];
			}
		}
		// �����_���ƌ듾�[���̊֌W�𒲂ׂ�
		for ( j = 0; j < eDbDocs; j++ ) {
			if ( j == max_doc )	continue;
			xs_sum += nums[j] * nums[j];
			xy_sum += nums[j] * score[j];
//			printf("%ld,%ld\n", xs_sum, xy_sum);
		}
		ReleaseNears( nears, nums[i] );	// nears�̉��
	}
//	printf("%lf\n", (double)xy_sum / (double)xs_sum);
	return (double)xy_sum / (double)xs_sum;
}

#if 0
double CalcProp( char *prop_path )
// ���萔�𒲂ׂ�
{
	int i, j, max_doc;
    int num, **nears = 0;
    int pn[kMaxDocNum], score[kMaxDocNum];
    long xs_sum = 0, xy_sum = 0;
    char line[1024];
#ifdef	WIN32
	WIN32_FIND_DATA fFind;
	HANDLE hSearch;
	char fname[kMaxPathLen];
#else
	glob_t gt;
#endif
	CvPoint *ps = 0;
	FILE *fp_pn;
	CvSize img_size;

	if ( *prop_path == '\0' )	return 0.0;	// prop_path���ݒ肳��Ă��Ȃ����0
	// �e�o�^�摜�̓����_���̓ǂݍ���
	if ( ( fp_pn = fopen(ePNFileName, "r") ) == NULL ) {
		fprintf(stderr, "%s open error\n", ePNFileName);
		return -1.0;
	}
//	fprintf(stderr, "%d\n", eDbDocs);
	for ( i = 0; i < eDbDocs; i++ ) {
//		fprintf(stderr, "%d\n", i);
		if ( fgets(line, 1024, fp_pn) == NULL ) {
			fprintf(stderr, "Error: point number file\n");
			return -1.0;
		}
		sscanf(line, "%d", &(pn[i]));
	}
	fclose( fp_pn );
#ifdef	WIN32
	hSearch = FindFirstFile(prop_path, &fFind); /* �T���J�n */
	if ( hSearch == INVALID_HANDLE_VALUE )	return 0.0;	// �}�b�`������̂��Ȃ����0
	do {
		printf("%s\n", fFind.cFileName);
		GetDir( prop_path, kMaxPathLen, fname );	// �f�B���N�g���̎擾
		strcat(fname, fFind.cFileName);	// ��΃p�X�̍쐬
		num = LoadPointFile( fname, &ps, &img_size );	// �����_�̓ǂݍ���
		if ( num == 0 ) {
			fprintf(stderr, "Error: no point in %s\n", fFind.cFileName);
			return -1.0;
		}
		MakeNearsFromCentres( ps, num, &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����
		max_doc = RetrieveNN5( ps, num, nears, img_size, score );	// �����̂�
		for ( j = 0; j < eDbDocs; j++ ) {
			if ( j == max_doc )	continue;
			xs_sum += pn[j] * pn[j];
			xy_sum += pn[j] * score[j];
		}
		ReleaseCentres( ps );	// ps�̉��
		ReleaseNears( nears, num );	// nears�̉��
	} while ( FindNextFile( hSearch, &fFind ) );	/* �S�t�@�C�������� */
#else
	glob(prop_path, GLOB_NOCHECK, errfunc2, &gt);
	for ( i = 0; (unsigned int)i < gt.gl_pathc; i++ ) {
		printf("%s\n", gt.gl_pathv[i]);
		
		num = LoadPointFile( gt.gl_pathv[i], &ps, &img_size );	// �����_�̓ǂݍ���
		if ( num == 0 ) {
			fprintf(stderr, "Error: no point in %s\n", gt.gl_pathv[i]);
			return -1.0;
		}
		MakeNearsFromCentres( ps, num, &nears );	// �e�_�ɂ��ď��kNears�̋ߖT�_�̃C���f�b�N�X�����߁Cnears�Ɋi�[����
		max_doc = RetrieveNN5( ps, num, nears, img_size, score );	// �����̂�
		for ( j = 0; j < eDbDocs; j++ ) {
			if ( j == max_doc )	continue;
			xs_sum += pn[j] * pn[j];
			xy_sum += pn[j] * score[j];
//			printf("%ld,%ld\n", xs_sum, xy_sum);
		}
		
		ReleaseCentres( ps );	// ps�̉��
		ReleaseNears( nears, num );	// nears�̉��
	}
	globfree(&gt);
#endif
	printf("%lf\n", (double)xy_sum / (double)xs_sum);
	return (double)xy_sum / (double)xs_sum;
}
#endif
