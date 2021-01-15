#define	WIN
//	#define	RESO_LIMIT
//	#define	FUNC_LIMIT

#ifdef	WIN
#pragma	warning(disable:4996)
#pragma	warning(disable:4819)
#pragma	warning(disable:4995)
#endif

#define	kFileNameLen	(64)
#define	kMaxPathLen	(1024)	/* パスの文字列の最大長 */
#define	kMaxLineLen	(1024)	/* fgetsで読み込む場合のバッファサイズ */

#ifndef M_PI /* 円周率 (pi) */
#define M_PI (3.14159265358979323846L)
#endif
