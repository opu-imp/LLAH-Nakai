/* グローバル変数を1箇所に集めるための定義 */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eNumCom1;	// nCm
Extern int eNumCom2;	// mC(f or f-1)
Extern int eGroup1Num;	// n: パラメータn
Extern int eGroup2Num;	// m: パラメータm
Extern int eGroup3Num;	// f or f-1
Extern int eDiscNum;	// d: 離散化レベル
Extern int eDbDocs;	// データベースのページ数
Extern int eInvType;	// 不変量のタイプ
Extern int eIncludeCentre;	// 中心点を含むか
Extern int eUseArea;	// 面積を用いるか
Extern char eHashSrcPath[kMaxPathLen];	// ハッシュの元画像ファイルの探索パス
Extern char ePFPrefix[kMaxPathLen];	// ハッシュの点ファイルのディレクトリ
Extern char ePFSuffix[kMaxPathLen];	// ハッシュの点ファイルの拡張子
Extern char eDBCorFileName[kMaxPathLen];	// 対応ファイル
Extern char eDiscFileName[kMaxPathLen];	// 離散化ファイル
Extern char ePNFileName[kMaxPathLen];	// 点の数のファイル
Extern char eHashFileName[kMaxPathLen]; // ハッシュのファイル名
Extern char eDirsDir[kMaxPathLen];	// 設定ファイルを保存するディレクトリ
Extern char ePropMakePath[kMaxPathLen];	// 比例定数を計算するための検索質問のパス
Extern double eProp;	// スコア補正の比例定数
Extern int eVectorCheck;	// ベクトルチェックするかどうか

// 以下、annex用
Extern char eOrigFileName[kMaxPathLen];
Extern char eAnnoFileName[kMaxPathLen];
Extern char eOutPutDir[kMaxPathLen];	// o: 出力ディレクトリ
Extern int eResizeMode;	// annoをリサイズするかどうか
Extern int eOrigMode;	// O: 比較対象をオリジナルPDFとするかどうか
Extern int eTransMode;	// A: 位置合わせ変換法（相似orアフィン）

// mainで縮小する際の倍率（0.5）
Extern double eResizeMainOrig;	// z: 元画像
Extern double eResizeMainAnno;	// Z: 書き込み画像
// 差分取得の準備
Extern int eOrigErodeIterMain;	// R: 元画像がPDFの場合、元画像をerodeするパラメータ
Extern int eOrigGaussParamMain;	// G: 元画像がスキャンの場合、元画像にGaussianをかけるパラメータ
Extern int eAnnoGaussPramMain;	// g: 書き込み画像にGaussianをかけるパラメータ

// 二値化する際の閾値
//Extern double eBinThrOrig;	// t: 元画像がPDFの場合
//Extern double eBinThrScan;	// u: 元画像がスキャンされたものの場合
Extern double eBinThr;	// t: 二値化する際の閾値

Extern int eClosingIter;	// h: ばらばらになった連結成分を結合する際のモルフォロジ演算のパラメータ
Extern double eMinAreaMain;	// M: mainでノイズ除去する際の最小面積
Extern int eDilMaskIter;	// D: マスクを太らせる際のパラメータ

// transform.c
Extern char eAcr2Path[kMaxPathLen];	// P: 不変量の離散化テーブルを作成するコマンドacr2のパス

// cspoints.c
Extern double eClScale;	// L: 特徴点抽出時に縮小する際の倍率
Extern int eErodeClIter;	// I: クラスタリングの前にErodeする際のパラメータ
Extern int eClCmpStep;	// T: クラスタリング前に縮小する際のステップ（4ならサイズは約1/4になる）
Extern int eKMeansMaxIter;	// i: クラスタリング時の最大繰り返し数
Extern double eKMeansEpsilon;	// p: クラスタリング時の精度パラメータ
Extern double eMinAreaCl;	// e: 特徴点抽出でのノイズ除去時の最小面積
Extern double eMaxAreaCl;	// E: 特徴点抽出でのノイズ除去時の最大面積

// annex_diff.c
Extern int eDiffNear;	// N: 最も近い画素を探す際の範囲
Extern double eDiffLeaveThr;	// l: 差分を取らずにそのまま残す際の閾値
Extern double eDiffEraseThr;	// H: 十分に近いとみなす際の閾値
