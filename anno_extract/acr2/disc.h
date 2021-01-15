#define	kDiscLineLen	(1024)	// disc.cにおけるfgetsで読む行の長さ

typedef struct _strDisc {	// 連続値の複比を離散値に変換するための構造体
	double min;	// 最小値
	double max;	// 最大値
	int num;	// 離散値の個数
	int res;	// datの個数
	int *dat;	// 離散化後の値を入れる配列
} strDisc;

int Con2DiscCR( double cr, strDisc *disc );
int Con2DiscCREq(double cr, strDisc *disc);
int LoadDisc( char *fname, strDisc *disc );
