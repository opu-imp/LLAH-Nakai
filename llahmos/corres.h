typedef struct _strCorList {
	struct _strCorList *next;
	int x;	// 見つかった画像の点座標
	int y;
	double dist;
} strCorList;

typedef struct _strCorMap {
	struct _strCorMap *next;
	int x;	// クエリー画像の点座標
	int y;
	int cor_num;	// 対応点の数
	strCorList *cor;	// 対応点リスト
} strCorMap;

void InitCorres( void );
strCorMap *MakePoint( int x, int y );
void AddCorPoint( strCorMap *m, int x, int y );
void AddPointTable( int idx, strCorMap *m );
void OutPutTable( void );
strCorMap *GetCM( int idx );
