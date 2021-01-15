/* グローバル変数を1箇所に集めるための定義 */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eCamNum;	// カメラの番号
Extern int eCamConfNum;	// カメラのモード
Extern int eTCPPort;	// TCP通信ポート
Extern int eProtocol;	// 通信プロトコル
Extern int ePointPort;	// 特徴点ポート（UDP）
Extern int eResultPort;	// 検索結果ポート（UDP）
Extern char eClientName[20];	// クライアントのマシン名
Extern char eServerName[20];	// サーバのマシン名
Extern int eEntireMode;	// 全体のモード
Extern char eMovieFileName[256];	// 動画のファイル名
Extern char eTuneFpRegFileName[256];	// チューニングモードでの登録画像名
Extern char eConvMovieFileName[256];	// 変換先動画のファイル名
Extern char eCamHarrisRegFileName[256];	// Harrisによる特徴点抽出モードでの登録画像名
Extern int eDetectHarrisCamMode;	// DetectHarrisCamのモード

Extern int eIsJp;	// 日本語モードかどうか
Extern int eJpAdpBlock;
Extern int eJpAdpSub;
Extern int eJpGParam;
Extern int eJpThr;

// 以下，nn5dcam用
#ifndef	kMaxPathLen
#define	kMaxPathLen	(128)	/* パスの文字列の最大長 */
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
Extern char eThumbDir[kMaxPathLen];	// サムネイルのディレクトリ
Extern char eThumbSuffix[kMaxPathLen];	// サムネイルの拡張子
Extern double eThumbScale;	// サムネイルの倍率

Extern char eHarrisTestOrigFileName[kMaxPathLen];
Extern char eHarrisTestAnnoFileName[kMaxPathLen];

Extern char eCopyright[kMaxLineLen];	// コピーライト
