typedef struct _strHist {
	int size;	// binの数
	int *bin;	// データ
	double min;	// データの最小値
	double max;	// データの最大値．minとmaxで入るbinを決める
	int max_freq;	// 頻度の最大値
} strHist;

void HistTest( void );
int InitHist( strHist *hist, int size, double min, double max );
void ReleaseHist( strHist *hist );
int AddDataHist( strHist *hist, double dat );
void OutPutHist( strHist *hist, int width, int height );
void DisplayHistNum( strHist *hist);
double GetMaxBin( strHist *hist );
void DrawHist( IplImage *img, strHist *hist );
