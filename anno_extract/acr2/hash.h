#define	DB_DOCS	(10000)

#define	kMaxNumCom1	(252)	/* eNumCom1�̍ő�l */
#define	kMaxNumCom2	(252)	/* eNumCom2�̍ő�l */
#define	kMaxGroup1Num	(10)	/* eGroup1Num�̍ő�l */
#define	kMaxGroup2Num	(10)		/* eGroup2Num�̍ő�l */
#define	kMaxGroup3Num	(5)		/* eGroup3Num�̍ő�l */

typedef struct _strHList {
	struct _strHList *next;
	unsigned short doc;
	unsigned short point;
//	unsigned char pat;
	char idx[kMaxNumCom2];
} strHList;

#define	USE_CON
#define	kHashSize	(1024*1024*128-1)	/* �n�b�V���e�[�u���̑傫�� */

#define	kMaxDocNum	(10240)	/* �o�^�ł���ő�h�L�������g�� */
#define	kExactDocNum	(10240)	/* ���ۂɓo�^����Ă���h�L�������g�� */
//#define	kMaxPointNum	(1024)	/* �_�̍ő吔 */
#define	kMaxPointNum	(2500)	/* �_�̍ő吔 */
#define	kMaxLineLen	(102400)
#define	kBlockSize	(1024)

#define	kCheckPPVarNum	(10)


strHList *ReadHash( char *index, int num );
void InitHash( void );
int AddHash( char *index, int num, unsigned short doc, unsigned short point );
int SearchHash( unsigned int index, unsigned int doc );
void PrintHash( void );
void SaveHash( void );
int LoadHash( int num );
int ChkHash( void );

static strHList **hash;
