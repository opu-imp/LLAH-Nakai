typedef struct _strHist {
	int size;	// bin�̐�
	int *bin;	// �f�[�^
	double min;	// �f�[�^�̍ŏ��l
	double max;	// �f�[�^�̍ő�l�Dmin��max�œ���bin�����߂�
	int max_freq;	// �p�x�̍ő�l
} strHist;

void HistTest( void );
int InitHist( strHist *hist, int size, double min, double max );
void ReleaseHist( strHist *hist );
int AddDataHist( strHist *hist, double dat );
void OutPutHist( strHist *hist, int width, int height );
void DisplayHistNum( strHist *hist);
double GetMaxBin( strHist *hist );
void DrawHist( IplImage *img, strHist *hist );
