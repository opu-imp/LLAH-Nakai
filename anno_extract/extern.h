/* �O���[�o���ϐ���1�ӏ��ɏW�߂邽�߂̒�` */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eNumCom1;	// nCm
Extern int eNumCom2;	// mC(f or f-1)
Extern int eGroup1Num;	// n: �p�����[�^n
Extern int eGroup2Num;	// m: �p�����[�^m
Extern int eGroup3Num;	// f or f-1
Extern int eDiscNum;	// d: ���U�����x��
Extern int eDbDocs;	// �f�[�^�x�[�X�̃y�[�W��
Extern int eInvType;	// �s�ϗʂ̃^�C�v
Extern int eIncludeCentre;	// ���S�_���܂ނ�
Extern int eUseArea;	// �ʐς�p���邩
Extern char eHashSrcPath[kMaxPathLen];	// �n�b�V���̌��摜�t�@�C���̒T���p�X
Extern char ePFPrefix[kMaxPathLen];	// �n�b�V���̓_�t�@�C���̃f�B���N�g��
Extern char ePFSuffix[kMaxPathLen];	// �n�b�V���̓_�t�@�C���̊g���q
Extern char eDBCorFileName[kMaxPathLen];	// �Ή��t�@�C��
Extern char eDiscFileName[kMaxPathLen];	// ���U���t�@�C��
Extern char ePNFileName[kMaxPathLen];	// �_�̐��̃t�@�C��
Extern char eHashFileName[kMaxPathLen]; // �n�b�V���̃t�@�C����
Extern char eDirsDir[kMaxPathLen];	// �ݒ�t�@�C����ۑ�����f�B���N�g��
Extern char ePropMakePath[kMaxPathLen];	// ���萔���v�Z���邽�߂̌�������̃p�X
Extern double eProp;	// �X�R�A�␳�̔��萔
Extern int eVectorCheck;	// �x�N�g���`�F�b�N���邩�ǂ���

// �ȉ��Aannex�p
Extern char eOrigFileName[kMaxPathLen];
Extern char eAnnoFileName[kMaxPathLen];
Extern char eOutPutDir[kMaxPathLen];	// o: �o�̓f�B���N�g��
Extern int eResizeMode;	// anno�����T�C�Y���邩�ǂ���
Extern int eOrigMode;	// O: ��r�Ώۂ��I���W�i��PDF�Ƃ��邩�ǂ���
Extern int eTransMode;	// A: �ʒu���킹�ϊ��@�i����or�A�t�B���j

// main�ŏk������ۂ̔{���i0.5�j
Extern double eResizeMainOrig;	// z: ���摜
Extern double eResizeMainAnno;	// Z: �������݉摜
// �����擾�̏���
Extern int eOrigErodeIterMain;	// R: ���摜��PDF�̏ꍇ�A���摜��erode����p�����[�^
Extern int eOrigGaussParamMain;	// G: ���摜���X�L�����̏ꍇ�A���摜��Gaussian��������p�����[�^
Extern int eAnnoGaussPramMain;	// g: �������݉摜��Gaussian��������p�����[�^

// ��l������ۂ�臒l
//Extern double eBinThrOrig;	// t: ���摜��PDF�̏ꍇ
//Extern double eBinThrScan;	// u: ���摜���X�L�������ꂽ���̂̏ꍇ
Extern double eBinThr;	// t: ��l������ۂ�臒l

Extern int eClosingIter;	// h: �΂�΂�ɂȂ����A����������������ۂ̃����t�H���W���Z�̃p�����[�^
Extern double eMinAreaMain;	// M: main�Ńm�C�Y��������ۂ̍ŏ��ʐ�
Extern int eDilMaskIter;	// D: �}�X�N�𑾂点��ۂ̃p�����[�^

// transform.c
Extern char eAcr2Path[kMaxPathLen];	// P: �s�ϗʂ̗��U���e�[�u�����쐬����R�}���hacr2�̃p�X

// cspoints.c
Extern double eClScale;	// L: �����_���o���ɏk������ۂ̔{��
Extern int eErodeClIter;	// I: �N���X�^�����O�̑O��Erode����ۂ̃p�����[�^
Extern int eClCmpStep;	// T: �N���X�^�����O�O�ɏk������ۂ̃X�e�b�v�i4�Ȃ�T�C�Y�͖�1/4�ɂȂ�j
Extern int eKMeansMaxIter;	// i: �N���X�^�����O���̍ő�J��Ԃ���
Extern double eKMeansEpsilon;	// p: �N���X�^�����O���̐��x�p�����[�^
Extern double eMinAreaCl;	// e: �����_���o�ł̃m�C�Y�������̍ŏ��ʐ�
Extern double eMaxAreaCl;	// E: �����_���o�ł̃m�C�Y�������̍ő�ʐ�

// annex_diff.c
Extern int eDiffNear;	// N: �ł��߂���f��T���ۂ͈̔�
Extern double eDiffLeaveThr;	// l: ��������炸�ɂ��̂܂܎c���ۂ�臒l
Extern double eDiffEraseThr;	// H: �\���ɋ߂��Ƃ݂Ȃ��ۂ�臒l
