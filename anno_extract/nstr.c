#include <stdio.h>
#include <string.h>
#include "nstr.h"

int AddSlash( char *path, int len )
// 文字列pathの末尾に/がなければ追加する
{
	int i;
	
	for ( i = 0; i < len && path[i] != '\0'; i++ )
		;
	// lenまで終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// 終端文字がlen-1にあり，追加できない場合
	if ( i == len - 1 && path[i-1] != '/' )	return 0;
	// すでに'/'がある場合
	if ( path[i-1] == '/' )	return 1;
	// '/'を追加
	path[i] = '/';
	path[i+1] = '\0';
	
	return 1;
}

int GetSuffix( char *path, int len, char *suff )
// パス文字列pathから拡張子を取得し，suffに格納する
{
	int i, last_dot;
	
	suff[0] = '\0';
	for ( i = 0, last_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == '.' )	last_dot = i;
	}
	// lenまで終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// ピリオドが見つからなかった場合
	if ( last_dot < 0 )	return 1;
	// 拡張子をsuffにコピー
	strcpy( suff, path+last_dot+1 );
	
	return 1;
}

int GetBasename( char *path, int len, char *basename )
// パス文字列pathからディレクトリ部分と拡張子を除いた文字列を取得し，basenameに格納する
{
	int i, last_slash, first_dot;
	
	basename[0] = '\0';
	for ( i = 0, last_slash = -1, first_dot = -1; i < len && path[i] != '\0'; i++ ) {
		if ( path[i] == '/' )	last_slash = i;
		if ( path[i] == '.' && first_dot < 0 )	first_dot = i;
	}
	// lenまで終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// ドットがなかった場合
	last_slash++;
	if ( first_dot < 0 )	first_dot = i - 1;
	strcpy( basename, path + last_slash );
	basename[first_dot - last_slash] = '\0';
	
	return 1;
}

int GetDir( char *path, int len, char *dir )
// パス文字列pathからディレクトリの文字列を取得し，dirに格納する
{
	int i, last_slash;
	
	for ( i = 0, last_slash = -1; i < len && path[i] != '\0'; i++ ) {
		dir[i] = path[i];
		if ( path[i] == '/' )	last_slash = i;
	}
	// lenまで終端文字が見つからなかった場合
	if ( i == len )	return 0;
	// スラッシュが見つからなかった場合、空文字列を返す
	if ( last_slash < 0 ) {
		dir[0] = '\0';
		return 1;
	}
	dir[last_slash + 1] = '\0';
	return 1;
}
