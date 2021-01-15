#include <stdio.h>
#include <stdlib.h>
#include "def_general.h"
#include "extern.h"
#include "hash.h"
#include "hlist2.h"

int SetBit( unsigned char *dst, int dpos, unsigned long src, int spos )
// ビットのセット
{
	unsigned char dat;
	
	// srcのsposビット目を取得
	dat = (src >> spos) & 0x0001;
	// dstのdposビット目をクリア
	dst[dpos >> 3] &= ~(0x0001 << (7 - dpos % 8));
	// dstのdposビット目にdatをセット
	dst[dpos >> 3] |= (dat << (7 - dpos % 8));
	
	return 1;
}

int SetBits( unsigned char *dst, int dfrom, unsigned long src, int len )
// ビットデータのコピー
// src側をunsigned long(8バイト）としたもの
// srcの下位からlenビットをdstのdfromからlenビットまでコピーする
{
	int i;
	
	for ( i = 0; i < len; i++ ) {
		SetBit( dst, dfrom + i, src, len - i - 1 );
	}
	return 1;
}

unsigned char GetBit( unsigned char *src, int pos )
// ビットの取得
{
	return ((src[pos >> 3]) >> (7 - pos % 8)) & 0x0001;
}

unsigned long GetBits( unsigned char *src, int from, int len )
// ビットデータの取得
{
	int i;
	unsigned char dat;
	unsigned long res = 0;
	
	for ( i = 0; i < len; i++ ) {
		dat = GetBit( src, from + i );
		res |= (dat << (len - i - 1));
	}
	
	return res;
}

int SetBits2( unsigned char *dst, int dfrom, unsigned long src, int len )
// ビットデータのコピー
// src側をunsigned long(8バイト）としたもの
// srcの下位からlenビットをdstのdfromからlenビットまでコピーする
{
	int i, cp_len, done_len;
	unsigned char dmask, smask;
	
	if ( (dfrom + len) % 8 > len ) {	// 8ビット未満でまたがない場合
		i = (int)((dfrom + len) / 8);	// 最初にコピーする場所
		smask = ((0x0001 << len) - 1) << (8 - ((dfrom + len) % 8));	// srcのマスク
		dmask = 0x00ff & ( ~smask );	// dstのマスク
		dst[i] = (dst[i] & dmask) | ((src << (8 - ((dfrom + len) % 8))) & smask);
		return 1;
	}
	// 8ビット以上の場合
	// 右端の処理
	i = (int)((dfrom + len) / 8);	// 最初にコピーする場所
	cp_len = (dfrom + len) % 8;	// ビット数
//	printf("%d, %d\n", i, cp_len);
	dmask = (0x0001 << (8 - cp_len)) - 1;	// dstのビットマスク
	smask = (0x0001 << cp_len) - 1;	// srcのビットマスク
	dst[i] = ( dst[i] & dmask ) | ( ( src & smask ) << ( 8 - cp_len ) );
	src = src >> cp_len;	// srcをシフト
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i--;	// セットする場所（バイト単位）を移動
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// 最大8ビット
//		printf("%d, %d\n", i, cp_len);
		dmask = 0x00ff & ~((0x0001 << cp_len) - 1);	// dstのビットマスク（上位）
		smask = (0x0001 << cp_len) - 1;	// srcのビットマスク
		dst[i] = ( dst[i] & dmask ) | ( src & smask );
		src = src >> cp_len;	// srcをシフト
	}
	return 1;
}

unsigned long GetBits2( unsigned char *src, int from, int len )
// ビットデータの取得
{
	int i, cp_len, done_len;
	unsigned char mask;
	unsigned long res = 0;
	
	if ( (from + len) % 8 > len ) {	// 8ビット未満でまたがない場合
		i = (int)((from + len) / 8);	// コピーする場所
		mask = (0x0001 << len) -1;	// マスク
		res = ( src[i] >> (8 - (from + len) % 8) ) & mask;	// シフトしてマスクとAND
		return res;
	}
	// 8ビット以上もしくはまたぐ場合
	// 左端の処理
	i = (int)(from / 8);	// 最初にコピーする場所
	cp_len = 8 - from % 8;	// ビット数
//	printf("%d, %d\n", i, cp_len);
	mask = (0x0001 << cp_len) - 1;	// srcのビットマスク
	res = src[i] & mask;	// マスクとAND
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i++;	// セットする場所（バイト単位）を移動
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// 最大8ビット
//		printf("%d, %d\n", i, cp_len);
		mask = (0x0001 << cp_len) - 1;
		res = (res << cp_len) | ((src[i] >> (8 - cp_len)) & mask);
	}
	return res;
}

unsigned char *MakeHList2Dat( unsigned long doc, unsigned long point, char *r, char *o )
// ハッシュリストを作成
// 07/07/05 面積特徴量の記録を追加
{
	int i;
	unsigned char *dat;
	
	dat = (unsigned char *)calloc( eHList2DatByte, sizeof(unsigned char) );
	SetBits2( dat, 0, doc, eDocBit );
	SetBits2( dat, eDocBit, point, ePointBit );
	if ( !eNoCompareInv ) {	// -vオプションが設定されていたら特徴量の保存は行わない
		for ( i = 0; i < eNumCom2; i++ ) {
			SetBits2( dat, eDocBit + ePointBit + eRBit*i, r[i], eRBit );
		}
		if ( eUseArea ) {	// 面積の使用
			for ( i = 0; i < eGroup2Num; i++ ) {
				SetBits2( dat, eDocBit + ePointBit + eRBit*eNumCom2 + eOBit*i, o[i], eOBit );
			}
		}
	}
	
	return dat;
}

int ReadHList2Dat( unsigned char *dat, unsigned long *pdoc, unsigned long *ppoint, char *r, char *o )
// ハッシュリストの読み込み
// 07/07/20 面積特徴量の読み込みを追加
{
	int i;
	
	*pdoc = GetBits2( dat, 0, eDocBit );
	*ppoint = GetBits2( dat, eDocBit, ePointBit );
	if ( !eNoCompareInv ) {	// -vオプションが設定されていたら特徴量の保存は行わない
		for ( i = 0; i < eNumCom2; i++ ) {
			r[i] = GetBits2( dat, eDocBit + ePointBit + eRBit*i, eRBit );
		}
		if ( eUseArea ) {	// 面積の使用
			for ( i = 0; i < eGroup2Num; i++ ) {
				o[i] = GetBits2( dat, eDocBit + ePointBit + eRBit*eNumCom2 + eOBit*i, eOBit );
			}
		}
	}
	
	return 1;
}
