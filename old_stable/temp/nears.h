typedef struct _strDiv {	// �����̈�
	int xindex;	// x���̃C���f�b�N�X
	int yindex;	// y���̃C���f�b�N�X
	double dist;	// ���ڂ��Ă�������_�Ƃ̍ŏ�����
} strDiv;

typedef struct _strPointDist {	// �\�[�g�̂��߂̓����_�Ƌ����̍\����
	int pindex;	// �_�̃C���f�b�N�X
	double	dist;	// �_�܂ł̋���
} strPointDist;

void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
void NearestPoint( int n, CvPoint *ps, int num, int nears[] );
void MakeNearsFromCentresDiv( CvPoint *ps, int pnum, CvSize *size, int tx, int ty, int kn, int ***nears0 );
double CalcDivDist( CvPoint pt1, CvSize *size, int tx, int ty, int x, int y, int div_x, int div_y );
void MergePointDist( strPointDist *tmp0, int t0num, strPointDist *tmp1, int t1num, int kn, strPointDist **merge0, int *mnum0 );
