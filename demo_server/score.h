typedef struct _strScore {
	int n;
	int vote;
} strScore;

int score_compare( const strScore *a, const strScore *b );
void SetAndSortScore( strScore *score, int *dat, int n );
