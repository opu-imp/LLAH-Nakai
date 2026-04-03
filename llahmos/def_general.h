//#define	WIN
#define	LINUX
//#define	RESO_LIMIT
// DLL
#ifdef	WIN	// ���݂�WIN�ł̂�
#define	LLAHDOC_EXPORTS
#endif

// �o�[�W����
#define	kLlahDocVersion	(1.1)

#ifdef	WIN
#define	WIN_TIME
#else	
#define LINUX_TIME
#endif

#ifndef	WIN
#define	SOCKET	int
#endif

#ifdef	WIN
#define	kMkdirNoMesOpt	""
#else
#define	kMkdirNoMesOpt	"-p"
#endif

#ifdef	WIN
#pragma	warning(disable:4996)
#pragma	warning(disable:4819)
#endif

#define	kFileNameLen	(64)
#define	kMaxPathLen	(128)	/* �p�X�̕�����̍ő咷 */
#define	kMaxLineLen	(1024)	/* fgets�œǂݍ��ޏꍇ�̃o�b�t�@�T�C�Y */

#ifndef M_PI /* �~���� (pi) */
#define M_PI (3.14159265358979323846L)
#endif
