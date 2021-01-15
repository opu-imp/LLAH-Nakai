#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def_general.h"
#include "fpath.h"
#include "nstr.h"
#ifdef	WIN32
#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#else
#include <glob.h>
#endif

int errfunc_fpath( const char *epath, int eerrno )
{
	fprintf(stderr, "%d : %s\n", eerrno, epath );
	return 1;
}

int FindPath( char search_path[kMaxPathLen], char ***files0 )
// パスを展開する
{
	int i, count, num, wlen;
	char **files;
#ifdef	WIN32
	char dir[kMaxPathLen], fname[kMaxPathLen];
	WIN32_FIND_DATA fFind;
	HANDLE hSearch;
	BOOL ret = TRUE;
	WCHAR wpath[kMaxPathLen];
	size_t convertedChars = 0;
#else
	glob_t gt;
	int ret;
#endif

#ifdef	WIN32
	GetDir( search_path, kMaxPathLen, dir );	// ディレクトリを取得
	mbstowcs_s( &convertedChars, wpath, strlen(search_path) + 1, search_path, _TRUNCATE );	// WCHARに変換
	hSearch = FindFirstFile( wpath, &fFind); /* 元画像ディレクトリの探索開始 */
	if ( hSearch == INVALID_HANDLE_VALUE ) {	// 検索に失敗
		fprintf( stderr, "error: %s matches no files\n", search_path );
		return 0;
	}
	// 数を数える
	num = 0;
	do {
		num++;
		ret = FindNextFile( hSearch, &fFind );
	} while ( ret == TRUE );
	FindClose( hSearch );
	// もう一度検索してファイル名を格納する
	hSearch = FindFirstFile( wpath, &fFind); /* 元画像ディレクトリの探索開始 */
	files = (char **)calloc( num, sizeof(char *) );
	for ( i = 0, count = 0; i < num; i++ ) {
		wcstombs_s( &convertedChars, fname, kMaxPathLen, fFind.cFileName, _TRUNCATE );	// とりあえず変換
		if ( convertedChars > 0 ) {	// 変換成功なら
			wlen = wcslen(fFind.cFileName);
			files[count] = (char *)calloc( strlen(dir) + wlen + 1, sizeof(char) );
			strcpy( files[count], dir );	// とりあえずディレクトリをコピー
			strcat( files[count], fname );	// ファイル名を結合
			count++;
		}
		else {	// 変換失敗
			fprintf( stderr, "warning: file name conversion failed\n" );
		}

		FindNextFile( hSearch, &fFind );
	}
	FindClose( hSearch );
	*files0 = files;
	return count;
#else
	ret = glob( search_path, 0, errfunc_fpath, &gt );	// globの開始
	if ( ret != 0 ) {
		fprintf( stderr, "error: glob() returned %d\n", search_path, ret );
		return 0;
	}
	num = (int)gt.gl_pathc;
	files = (char **)calloc( num, sizeof(char *) );
	for ( i = 0; i < num; i++ ) {
		files[i] = (char *)calloc( strlen(gt.gl_pathv[i]) + 1, sizeof(char) );
		strcpy( files[i], gt.gl_pathv[i] );
	}
	globfree( &gt );
#endif
	*files0 = files;
	return num;
}
