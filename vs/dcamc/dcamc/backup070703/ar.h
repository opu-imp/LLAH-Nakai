//#define	kARDir		"\\\\leo\\nakai\\dirs\\ar\\"	/* AR情報のディレクトリ */
//#define	kARImageDir	"\\\\leo\\nakai\\dirs\\ar\\image\\"	/* AR情報の画像ディレクトリ */
#define	kARDir		"C:\\data\\dirs\\ar\\"	/* AR情報のディレクトリ */
#define	kARImageDir	"C:\\data\\dirs\\ar\\image\\"	/* AR情報の画像ディレクトリ */
#define	kMaxAR	(10)	/* AR情報の最大値 */

#define	kARTypeImg	'i'
#define	kARTypeRect	'r'
#define	kARTypeUL	'u'

typedef struct _strAR {	// 拡張現実情報の構造体
	char type;	// 種別
	CvPoint e[2];	// 端点
	IplImage *img;	// 画像
} strAR;

void DrawAR( IplImage *img_cap, char *doc_name, strProjParam param );
