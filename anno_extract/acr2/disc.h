#define	kDiscLineLen	(1024)	// disc.c�ɂ�����fgets�œǂލs�̒���

typedef struct _strDisc {	// �A���l�̕���𗣎U�l�ɕϊ����邽�߂̍\����
	double min;	// �ŏ��l
	double max;	// �ő�l
	int num;	// ���U�l�̌�
	int res;	// dat�̌�
	int *dat;	// ���U����̒l������z��
} strDisc;

int Con2DiscCR( double cr, strDisc *disc );
int Con2DiscCREq(double cr, strDisc *disc);
int LoadDisc( char *fname, strDisc *disc );
