// タスクバーの高さ・ウィンドウの枠
#define	kTaskBarHeight	(28)
#define	kCvWindowLeftMargin	(7)
#define	kCvWindowRightMargin	(6)
#define	kCvWindowTopMargin	(110)	/* トラックバーが2つある場合 */
#define	kCvWindowBottomMargin	(6)
// 登録画像のデフォルト値
#define	kDefaultTuneRegGParam1	(1)
#define	kDefaultTuneRegAdpBlock	(50)
#define	kDefaultTuneRegAdpSub	(10)
#define	kDefaultTuneRegGParam2	(1)
#define	kDefaultTuneRegThr		(200)
// キャプチャ画像のデフォルト値
#define	kDefaultTuneCapAdpBlock	(50)
#define	kDefaultTuneCapAdpSub	(10)
#define	kDefaultTuneCapGParam	(1)
#define	kDefaultTuneCapThr		(200)
// 登録画像のスライダの最大値
#define	kTuneGParam1Max	(20)
#define	kTuneAdpBlockMax	(100)
#define	kTuneAdpSubMax	(50)
#define	kTuneGParam2Max	(20)
#define	kTuneThrMax	(255)
// キャプチャ画像のスライダの最大値
#define	kTuneCapAdpBlockMax	(100)
#define	kTuneCapAdpSubMax	(50)
#define	kTuneCapGParamMax	(20)
#define	kTuneCapThrMax		(255)
// 特徴点描画
#define	kTuneRegPtRadius	(5)
#define	kTuneCapPtRadius	(5)
// モード
#define	SHOW_REG_IMG	(0)
#define	SHOW_REG_SMT1	(1)
#define	SHOW_REG_BIN1	(2)
#define	SHOW_REG_SMT2	(3)
#define	SHOW_REG_BIN2	(4)
#define	SHOW_REG_PT		(5)
#define	SHOW_REG_OVER	(6)
#define	SHOW_REG_COR	(7)

#define	SHOW_CAP_IMG	(0)
#define	SHOW_CAP_BIN1	(1)
#define	SHOW_CAP_SMT	(2)
#define	SHOW_CAP_BIN2	(3)
#define	SHOW_CAP_PT		(4)
#define	SHOW_CAP_OVER		(5)
#define	SHOW_CAP_COR		(6)

int TuneFeaturePointParam( char *fname );
