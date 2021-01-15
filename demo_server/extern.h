/* グローバル変数を1箇所に集めるための定義 */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eNumCom1;	// nCm
Extern int eNumCom2;	// mC(f or f-1)
Extern int eGroup1Num;	// パラメータn
Extern int eGroup2Num;	// パラメータm
Extern int eGroup3Num;	// f or f-1
Extern int eDiscNum;	// 離散化レベル
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
Extern int eCPF2StartNum;	// 特徴点抽出モード２での開始番号
Extern int eIsJp;	// 日本語モードかどうか
Extern int eExperimentMode;	// 実験モード（ハッシュサイズ・処理時間・精度の出力）
Extern int eCompressHash;	// ハッシュ圧縮モード

// iniファイル関係
Extern int eTCPPort;	// TCP通信ポート
Extern int eProtocol;	// 通信プロトコル
Extern int ePointPort;	// 特徴点ポート（UDP）
Extern int eResultPort;	// 検索結果ポート（UDP）
Extern char eClientName[20];	// クライアントのマシン名
Extern char eServerName[20];	// サーバのマシン名

// 特徴点抽出・日本語モード
Extern int eJpGParam1;
Extern int eJpAdpBlock;
Extern int eJpAdpSub;
Extern int eJpGParam2;
Extern int eJpThr;

// 打ち切り関係
Extern int eTerminate;	// 打ち切り方法
Extern int eTermVNum;	// 得票数での閾値

// 公開に向けて改変
Extern char eHashSrcDir[kMaxPathLen];	// ハッシュの元画像ファイルのディレクトリ
Extern char eHashSrcSuffix[kMaxPathLen];	// ハッシュの元画像ファイルの拡張子
Extern char ePointDatFileName[kMaxPathLen];	// point.datのファイル名
Extern char eHashDatFileName[kMaxPathLen];	// hash.datのファイル名
Extern int eEntireMode;	// 全体のモード
Extern int eHashGaussMaskSize;	// 電子文書の特徴点抽出のガウシアンフィルタのサイズ
Extern int eConnectHashImageThr;	// 電子文書の特徴点抽出の2値化の閾値
Extern int eDocNumForMakeDisc;	// 離散化ファイルの作成に用いる特徴点ファイルの数
Extern int ePropMakeNum;	// 比例定数を調べる際に用いる文書数
Extern char eConfigFileName[kMaxPathLen];	// config.datのファイル名
Extern char eCopyright_2006_TomohiroNakai_IMP_OPU[kMaxLineLen];	// コピーライト

// inacr2用
Extern double eDiscMin;
Extern double eDiscMax;

// ハッシュ圧縮モード関係
Extern int eDocBit;	// docのビット数
Extern int ePointBit;	// pointのビット数
Extern int eRBit;	// 不変量1つのビット数
Extern int eOBit;	// 面積特徴量1つのビット数
Extern int eHList2DatByte;	// 圧縮リストのデータ部のバイト数

// 回転の総当たりを行わない
Extern int eRotateOnce;

// 衝突の多いものをハッシュから除去
Extern int eRemoveCollision;
Extern int eMaxHashCollision;

// 特徴量の比較を行わない
Extern int eNoCompareInv;

// ハッシュテーブルに直接記録する（特徴量もなし）
Extern int eNoHashList;

// 多言語モード
Extern int eVariousAutoMaskSize;
Extern int eVariousAutoMaskSizeDual;

// デュアルハッシュモード（未使用？）
Extern int eDualHashMode;

// 複数ハッシュモード時のハッシュの数（未使用）
Extern int eHashNum;
