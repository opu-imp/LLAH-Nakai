#ifndef _LLAHDOC_DISC_H_
#define _LLAHDOC_DISC_H_

#define	kDiscLineLen	(1024)	// disc.c�ɂ�����fgets�œǂލs�̒���
#define	kDiscNum	(50)	/* ���U�����x���� */
#define	kMinAng	(0.0)	/* �p�x�̍ŏ��l */
#define	kMaxAng	(3.141592 * 2.0)	/* �p�x�̍ő�l */

typedef struct _strDisc {	// �A���l�̕���𗣎U�l�ɕϊ����邽�߂̍\����
	double min;	// �ŏ��l
	double max;	// �ő�l
	int num;	// ���U�l�̌�
	int res;	// dat�̌�
	int *dat;	// ���U����̒l������z��
} strDisc;

int Con2DiscCR( double cr, strDisc *disc );
int Con2DiscAngle( double cr );
int Con2DiscCREq(double cr, strDisc *disc);
int LoadDisc( char *fname, strDisc *disc );

#endif