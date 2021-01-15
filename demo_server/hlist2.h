int SetBit( unsigned char *dst, int dpos, unsigned long src, int spos );
int SetBits( unsigned char *dst, int dfrom, unsigned long src, int len );
int SetBits2( unsigned char *dst, int dfrom, unsigned long src, int len );
unsigned char GetBit( unsigned char *src, int pos );
unsigned long GetBits( unsigned char *src, int from, int len );
unsigned long GetBits2( unsigned char *src, int from, int len );
unsigned char *MakeHList2Dat( unsigned long doc, unsigned long point, char *r, char *o );
int ReadHList2Dat( unsigned char *dat, unsigned long *pdoc, unsigned long *ppoint, char *r, char *o );
