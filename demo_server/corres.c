#include "def_general.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hash.h"
#include "corres.h"

strCorMap *cmap[kMaxDocNum];

void InitCorres( void )
// �Ή��_�e�[�u��������������
{
	int i;

	for ( i = 0; i < kMaxDocNum; i++ )
		cmap[i] = NULL;
}

strCorMap *MakePoint( int x, int y )
// �Ή��_�e�[�u���̓_���쐬����
{
	strCorMap *m;

	m = (strCorMap *)calloc(1, sizeof(strCorMap));
	m->next = NULL;
	m->x = x;
	m->y = y;
	m->cor_num = 0;
	m->cor = NULL;
	
	return m;
}

void AddCorPoint( strCorMap *m, int x, int y )
// �_�ɑΉ��_��ǉ�����i�Ȃ�̂�������j
{
	strCorList *l;

	l = (strCorList *)calloc(1, sizeof(strCorList));
	l->next = m->cor;
	l->x = x;
	l->y = y;
	l->dist = sqrt((m->x - x)*(m->x - x) + (m->y - y)*(m->y - y));
	m->cor = l;
	m->cor_num++;
}

void AddPointTable( int idx, strCorMap *m )
// �e�[�u���ɓ_��ǉ�����
{
	m->next = cmap[idx];
	cmap[idx] = m;
}

strCorMap *GetCM( int idx )
// �e�[�u������擪�̓_�𓾂�
{
	return cmap[idx];
}

void OutPutTable( void )
// �e�[�u�����o�͂���
{
	int i;
	char fname[1024];
	FILE *fp;
	strCorList *cl;
	strCorMap *cm;

	for ( i = 0; i < kMaxDocNum; i++ ) {
		if ( cmap[i] == NULL )	continue;
		sprintf(fname, "point%03d.txt", i);
		fp = fopen(fname, "w");
		for ( cm = cmap[i]; cm != NULL; cm = cm->next ) {
			if ( cm->cor_num > 1 )	continue;
			for ( cl = cm->cor; cl != NULL; cl = cl->next ) {
//				printf("[%03d](%d,%d):%f\n", i, cl->x, cl->y, cl->dist);
				fprintf(fp, "%d %d %f\n", cl->x, cl->y, cl->dist);
			}
		}
		fclose(fp);
	}
}
