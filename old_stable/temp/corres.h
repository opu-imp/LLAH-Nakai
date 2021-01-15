typedef struct _strCorList {
	struct _strCorList *next;
	int x;	// ���������摜�̓_���W
	int y;
	double dist;
} strCorList;

typedef struct _strCorMap {
	struct _strCorMap *next;
	int x;	// �N�G���[�摜�̓_���W
	int y;
	int cor_num;	// �Ή��_�̐�
	strCorList *cor;	// �Ή��_���X�g
} strCorMap;

void InitCorres( void );
strCorMap *MakePoint( int x, int y );
void AddCorPoint( strCorMap *m, int x, int y );
void AddPointTable( int idx, strCorMap *m );
void OutPutTable( void );
strCorMap *GetCM( int idx );
