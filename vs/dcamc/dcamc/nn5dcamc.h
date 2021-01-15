void ConstructHashSub( CvPoint *ps, int num );
void ConstructHashSubSub(CvPoint *ps, int num, int *nears[], int n, strDisc *disc);
void NearestPoint( int n, CvPoint *ps, int num, int nears[] );
void MakeNearsFromCentres( CvPoint *ps, int num, int ***nears0 );
void CalcOrderCWN( int pt, CvPoint *ps, int *nears[], int idx[], int num );
void CalcInterPoint( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint *p1234 );
double CalcCR5( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4, CvPoint p5 );
double CalcCR( CvPoint p1, CvPoint p2, CvPoint p3, CvPoint p4 );
void InitCom( int ***com0, int num1, int num2 );
void GenerateCombination( int inv_type, int g1, int g2, int g3, void (*setcom1)(int, int, int), void (*setcom2)(int, int, int));
int RetrieveNN5(CvPoint *ps, int num, int *nears[], CvSize size, int *score, int *pcor );
void CalcScore3(CvPoint *ps, int num, int *nears[], int *score, strDisc *disc);
void VoteDirectlyByHL( strHList *hl, int *score, char *idx, int p );
void ClearFlag1(void);
void ClearCorFlag( void );

