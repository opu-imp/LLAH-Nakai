#include <stdio.h>
#include <string.h>
#include "def_general.h"
#include "nstr.h"

#define	kDelimiter	'/'
//#define	kDelimiter	'\\'

int AddSlash( char *path, int len )
// ������path�̖�����/���Ȃ���Βǉ�����
{
	int i;
	
	for ( i = 0; i < len && path[i] != '\0'; i++ )
		;
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �I�[������len-1�ɂ���C�ǉ��ł��Ȃ��ꍇ
	if ( i == len - 1 && path[i-1] != kDelimiterChar )	return 0;
	// ���ł�'/'������ꍇ
	if ( path[i-1] == kDelimiterChar )	return 1;
	// '/'��ǉ�
	path[i] = kDelimiterChar;
	path[i+1] = '\0';
	
	return 1;
}

int GetSuffix( char *path, int len, char *suff )
// �p�X������path����g���q���擾���Csuff�Ɋi�[����
{
	int i, last_dot;
	
	suff[0] = '\0';
	for ( i = 0, last_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == '.' )	last_dot = i;
	}
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �s���I�h��������Ȃ������ꍇ
	if ( last_dot < 0 )	return 1;
	// �g���q��suff�ɃR�s�[
	strcpy( suff, path+last_dot+1 );
	
	return 1;
}

int GetBasename( char *path, int len, char *basename )
// �p�X������path����f�B���N�g�������Ɗg���q����������������擾���Cbasename�Ɋi�[����
{
	int i, last_slash, first_dot;
	
	basename[0] = '\0';
	for ( i = 0, last_slash = -1, first_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == kDelimiterChar )	last_slash = i;
		if ( path[i] == '.' && first_dot < 0 )	first_dot = i;
	}
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �h�b�g���Ȃ������ꍇ
	last_slash++;
	if ( first_dot < 0 )	first_dot = i - 1;
	strcpy( basename, path + last_slash );
	basename[first_dot - last_slash] = '\0';
	
	return 1;
}

int GetBasename2( char *path, int len, char *basename )
// �p�X������path����f�B���N�g�������Ɗg���q����������������擾���Cbasename�Ɋi�[����
/*
/home/nakai/aaa.dat -> aaa
/home/nakai/aaa -> aaa
/home/nakai/aaa.dat.bak -> aaa
./aaa.dat -> aaa
../../aaa -> aaa
/home/nakai/.dir/aaa -> aaa
/home/nakai/.aaa -> �Ȃ�
�O���瑖�����Ă����āC�f���~�^��������last_slash���X�V�D�h�b�g�͏���̂�first_dot���X�V�D
�f���~�^�������Ƃ���first_dot���L���Ȃ疳���ɂ���
*/

{
	int i, last_slash, first_dot;
	
	basename[0] = '\0';
	for ( i = 0, last_slash = -1, first_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == kDelimiterChar ) {
			last_slash = i;
			first_dot = -1;	// �f���~�^�̑O�̃h�b�g�͖���������
		}
		if ( path[i] == '.' && first_dot < 0 )	first_dot = i;
	}
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	last_slash++;
	if ( first_dot < 0 )	first_dot = i - 1;	// �h�b�g���Ȃ������ꍇ�C�������I�[�Ƃ���
	strcpy( basename, path + last_slash );
	basename[first_dot - last_slash] = '\0';
	
	return 1;
}

int GetDir( char *path, int len, char *dir )
// �p�X������path����f�B���N�g���̕�������擾���Cdir�Ɋi�[����
{
	int i, last_slash;
	
	for ( i = 0, last_slash = -1; i < len && path[i] != '\0'; i++ ) {
		dir[i] = path[i];
		if ( path[i] == kDelimiterChar )	last_slash = i;
	}
	// len�܂ŏI�[������������Ȃ������ꍇ
	if ( i == len )	return 0;
	// �X���b�V����������Ȃ������ꍇ�A�󕶎����Ԃ�
	if ( last_slash < 0 ) {
		dir[0] = '\0';
		return 1;
	}
	dir[last_slash + 1] = '\0';
	return 1;
}
