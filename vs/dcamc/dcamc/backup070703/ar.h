//#define	kARDir		"\\\\leo\\nakai\\dirs\\ar\\"	/* AR���̃f�B���N�g�� */
//#define	kARImageDir	"\\\\leo\\nakai\\dirs\\ar\\image\\"	/* AR���̉摜�f�B���N�g�� */
#define	kARDir		"C:\\data\\dirs\\ar\\"	/* AR���̃f�B���N�g�� */
#define	kARImageDir	"C:\\data\\dirs\\ar\\image\\"	/* AR���̉摜�f�B���N�g�� */
#define	kMaxAR	(10)	/* AR���̍ő�l */

#define	kARTypeImg	'i'
#define	kARTypeRect	'r'
#define	kARTypeUL	'u'

typedef struct _strAR {	// �g���������̍\����
	char type;	// ���
	CvPoint e[2];	// �[�_
	IplImage *img;	// �摜
} strAR;

void DrawAR( IplImage *img_cap, char *doc_name, strProjParam param );
