void ConstructHashSub( cv::Point *ps, int num );
void ConstructHashSubSub(cv::Point *ps, int num, int *nears[], int n, strDisc *disc);
void NearestPoint( int n, cv::Point *ps, int num, int nears[] );
void MakeNearsFromCentres( cv::Point *ps, int num, int ***nears0 );
void CalcOrderCWN( int pt, cv::Point *ps, int *nears[], int idx[], int num );
void CalcInterPoint( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point *p1234 );
double CalcCR5( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4, cv::Point p5 );
double CalcCR( cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4 );
void InitCom( int ***com0, int num1, int num2 );
void GenerateCombination( int inv_type, int g1, int g2, int g3, void (*setcom1)(int, int, int), void (*setcom2)(int, int, int));
int RetrieveNN5(cv::Point *ps, int num, int *nears[], cv::Size size, int *score, int *pcor );
void CalcScore3(cv::Point *ps, int num, int *nears[], int *score, strDisc *disc);
void VoteDirectlyByHL( strHList *hl, int *score, char *idx, int p );
void ClearFlag1(void);
void ClearCorFlag( void );

