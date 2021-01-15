#define	kIniFileName	"server.ini"
#define	kDefaultTCPPort		(12345)
#define	kDefaultProtocol	(2)	/* 1はTCP，2はUDP */
#define	kDefaultPointPort	(65431)
#define	kDefaultResultPort	(65432)
#define	kDefaultClientName	"localhost"
#define	kDefaultServerName	"localhost"

// 公開に向けて改変
#define	kDfltDirsDir	"./"
#define	kDfltHashSrcDir	"./"
#define	kDfltHashSrcSuffix	"bmp"
#define	kDfltPointDatFileName	"point.dat"
#define	kDfltHashDatFileName	"hash.dat"
#define	kDfltConfigFileName	"config.dat"

#define	kCopyright	"Copyright (C) 2006 Tomohiro Nakai, Intelligent Media Processing Laboratory, Osaka Prefecture University"

// 特徴点抽出
//#define	kDfltHashGaussMaskSize	(15)
// 最軽量の特徴点抽出のために変更　07/06/07
//#define	kDfltHashGaussMaskSize	(5)
// MIRU08の再実験のため変更　08/05/30
#define	kDfltHashGaussMaskSize	(7)

// 特徴量計算
#define	kDfltInvType	AFFINE
#define	kDfltGroup1Num	(7)
#define	kDfltGroup2Num	(6)
#define	kDfltDiscNum	(15)
#define	kDfltTerminate	TERM_NON
#define	kDfltIncludeCentre	0

// 離散化
#define	kDfltDiscFileName	"disc.txt"
#define	kDfltDocNumForMakeDisc	(10)

// 比例定数
#define	kDfltPropMakeNum	(10)

// 一般
#define	kConfigVerStr	"<config.dat 1.0>"

// ハッシュ圧縮
#define	kCompressHashDiscroptor	"Compress Hash"
#define	kNormalHashDiscroptor	"Normal Hash"

int AnalyzeArgAndSetExtern( int argc, char *argv[], int *p_emode, int *p_rmode, int *p_fmode, int *p_pmode );
int AnalyzeArgAndSetExtern2( int argc, char *argv[] );
int SetExtern( int emode, int n, int m, int inv_type, int disc_num, double prop, int inc_centre, int use_area, int vec_chk, \
 char *hs_path, char *dirs_dir, char *pf_pref, char *pf_suf, char *cor_fn, \
 char *disc_dir, char *dm_path, char *prop_path, char *pn_fn, char *hash_fn, int dbdocs, int start_num );
int SaveSetting( int inv_type, int n, int m, int d, char *point_dir, char *dbcor_fn, char *pn_fn );
int LoadSetting( int *inv_type, int *n, int *m, int *d, char *point_dir, char *dbcor_fn, char *pn_fn, double *p, int *dbdocs );
int ReadIniFile( void );
//int SaveConfig( void );
int SaveConfig( int inv_type, int group1_num, int group2_num, int disc_num, double prop, int compress_hash, int doc_bit, int point_bit, int r_bit, int o_bit, int hlist2_dat_byte, int rotate_once, int use_area, int is_jp, int no_hash_list,  const char *config_file_name );
//int LoadConfig( void );
int LoadConfig( const char *config_file_name, int *p_inv_type, int *p_group1_num, int *p_group2_num, int *p_group3_num, int *p_num_com1, int *p_num_com2, int *p_disc_num, double *p_prop, int *p_compress_hash, int *p_doc_bit, int *p_point_bit, int *p_r_bit, int *p_o_bit, int *p_hlist2_dat_byte, int *p_rotate_once, int *p_use_area, int *p_is_jp, int *p_no_hash_list );
