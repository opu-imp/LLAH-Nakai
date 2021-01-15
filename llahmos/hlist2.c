#include <stdio.h>
#include <stdlib.h>
#include "def_general.h"
#include "extern.h"
#include "hash.h"
#include "hlist2.h"

int SetBit( unsigned char *dst, int dpos, unsigned long src, int spos )
// �r�b�g�̃Z�b�g
{
	unsigned char dat;
	
	// src��spos�r�b�g�ڂ��擾
	dat = (src >> spos) & 0x0001;
	// dst��dpos�r�b�g�ڂ��N���A
	dst[dpos >> 3] &= ~(0x0001 << (7 - dpos % 8));
	// dst��dpos�r�b�g�ڂ�dat���Z�b�g
	dst[dpos >> 3] |= (dat << (7 - dpos % 8));
	
	return 1;
}

int SetBits( unsigned char *dst, int dfrom, unsigned long src, int len )
// �r�b�g�f�[�^�̃R�s�[
// src����unsigned long(8�o�C�g�j�Ƃ�������
// src�̉��ʂ���len�r�b�g��dst��dfrom����len�r�b�g�܂ŃR�s�[����
{
	int i;
	
	for ( i = 0; i < len; i++ ) {
		SetBit( dst, dfrom + i, src, len - i - 1 );
	}
	return 1;
}

unsigned char GetBit( unsigned char *src, int pos )
// �r�b�g�̎擾
{
	return ((src[pos >> 3]) >> (7 - pos % 8)) & 0x0001;
}

unsigned long GetBits( unsigned char *src, int from, int len )
// �r�b�g�f�[�^�̎擾
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
// �r�b�g�f�[�^�̃R�s�[
// src����unsigned long(8�o�C�g�j�Ƃ�������
// src�̉��ʂ���len�r�b�g��dst��dfrom����len�r�b�g�܂ŃR�s�[����
{
	int i, cp_len, done_len;
	unsigned char dmask, smask;
	
	if ( (dfrom + len) % 8 > len ) {	// 8�r�b�g�����ł܂����Ȃ��ꍇ
		i = (int)((dfrom + len) / 8);	// �ŏ��ɃR�s�[����ꏊ
		smask = ((0x0001 << len) - 1) << (8 - ((dfrom + len) % 8));	// src�̃}�X�N
		dmask = 0x00ff & ( ~smask );	// dst�̃}�X�N
		dst[i] = (dst[i] & dmask) | ((src << (8 - ((dfrom + len) % 8))) & smask);
		return 1;
	}
	// 8�r�b�g�ȏ�̏ꍇ
	// �E�[�̏���
	i = (int)((dfrom + len) / 8);	// �ŏ��ɃR�s�[����ꏊ
	cp_len = (dfrom + len) % 8;	// �r�b�g��
//	printf("%d, %d\n", i, cp_len);
	dmask = (0x0001 << (8 - cp_len)) - 1;	// dst�̃r�b�g�}�X�N
	smask = (0x0001 << cp_len) - 1;	// src�̃r�b�g�}�X�N
	dst[i] = ( dst[i] & dmask ) | ( ( src & smask ) << ( 8 - cp_len ) );
	src = src >> cp_len;	// src���V�t�g
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i--;	// �Z�b�g����ꏊ�i�o�C�g�P�ʁj���ړ�
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// �ő�8�r�b�g
//		printf("%d, %d\n", i, cp_len);
		dmask = 0x00ff & ~((0x0001 << cp_len) - 1);	// dst�̃r�b�g�}�X�N�i��ʁj
		smask = (0x0001 << cp_len) - 1;	// src�̃r�b�g�}�X�N
		dst[i] = ( dst[i] & dmask ) | ( src & smask );
		src = src >> cp_len;	// src���V�t�g
	}
	return 1;
}

unsigned long GetBits2( unsigned char *src, int from, int len )
// �r�b�g�f�[�^�̎擾
{
	int i, cp_len, done_len;
	unsigned char mask;
	unsigned long res = 0;
	
	if ( (from + len) % 8 > len ) {	// 8�r�b�g�����ł܂����Ȃ��ꍇ
		i = (int)((from + len) / 8);	// �R�s�[����ꏊ
		mask = (0x0001 << len) -1;	// �}�X�N
		res = ( src[i] >> (8 - (from + len) % 8) ) & mask;	// �V�t�g���ă}�X�N��AND
		return res;
	}
	// 8�r�b�g�ȏ�������͂܂����ꍇ
	// ���[�̏���
	i = (int)(from / 8);	// �ŏ��ɃR�s�[����ꏊ
	cp_len = 8 - from % 8;	// �r�b�g��
//	printf("%d, %d\n", i, cp_len);
	mask = (0x0001 << cp_len) - 1;	// src�̃r�b�g�}�X�N
	res = src[i] & mask;	// �}�X�N��AND
	for ( done_len = cp_len; done_len < len; done_len += cp_len ) {
		i++;	// �Z�b�g����ꏊ�i�o�C�g�P�ʁj���ړ�
		cp_len = len - done_len;
		if ( cp_len > 8 )	cp_len = 8;	// �ő�8�r�b�g
//		printf("%d, %d\n", i, cp_len);
		mask = (0x0001 << cp_len) - 1;
		res = (res << cp_len) | ((src[i] >> (8 - cp_len)) & mask);
	}
	return res;
}

unsigned char *MakeHList2Dat( unsigned long doc, unsigned long point, char *r, char *o )
// �n�b�V�����X�g���쐬
// 07/07/05 �ʐϓ����ʂ̋L�^��ǉ�
{
	int i;
	unsigned char *dat;
	
	dat = (unsigned char *)calloc( eHList2DatByte, sizeof(unsigned char) );
	SetBits2( dat, 0, doc, eDocBit );
	SetBits2( dat, eDocBit, point, ePointBit );
	if ( !eNoCompareInv ) {	// -v�I�v�V�������ݒ肳��Ă���������ʂ̕ۑ��͍s��Ȃ�
		for ( i = 0; i < eNumCom2; i++ ) {
			SetBits2( dat, eDocBit + ePointBit + eRBit*i, r[i], eRBit );
		}
		if ( eUseArea ) {	// �ʐς̎g�p
			for ( i = 0; i < eGroup2Num; i++ ) {
				SetBits2( dat, eDocBit + ePointBit + eRBit*eNumCom2 + eOBit*i, o[i], eOBit );
			}
		}
	}
	
	return dat;
}

int ReadHList2Dat( unsigned char *dat, unsigned long *pdoc, unsigned long *ppoint, char *r, char *o )
// �n�b�V�����X�g�̓ǂݍ���
// 07/07/20 �ʐϓ����ʂ̓ǂݍ��݂�ǉ�
{
	int i;
	
	*pdoc = GetBits2( dat, 0, eDocBit );
	*ppoint = GetBits2( dat, eDocBit, ePointBit );
	if ( !eNoCompareInv ) {	// -v�I�v�V�������ݒ肳��Ă���������ʂ̕ۑ��͍s��Ȃ�
		for ( i = 0; i < eNumCom2; i++ ) {
			r[i] = GetBits2( dat, eDocBit + ePointBit + eRBit*i, eRBit );
		}
		if ( eUseArea ) {	// �ʐς̎g�p
			for ( i = 0; i < eGroup2Num; i++ ) {
				o[i] = GetBits2( dat, eDocBit + ePointBit + eRBit*eNumCom2 + eOBit*i, eOBit );
			}
		}
	}
	
	return 1;
}
