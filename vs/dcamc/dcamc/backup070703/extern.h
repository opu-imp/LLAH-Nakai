/* �O���[�o���ϐ���1�ӏ��ɏW�߂邽�߂̒�` */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eCamNum;	// �J�����̔ԍ�
Extern int eCamConfNum;	// �J�����̃��[�h
Extern int eTCPPort;	// TCP�ʐM�|�[�g
Extern int eProtocol;	// �ʐM�v���g�R��
Extern int ePointPort;	// �����_�|�[�g�iUDP�j
Extern int eResultPort;	// �������ʃ|�[�g�iUDP�j
Extern char eClientName[20];	// �N���C�A���g�̃}�V����
Extern char eServerName[20];	// �T�[�o�̃}�V����
Extern int eEntireMode;	// �S�̂̃��[�h
Extern char eMovieFileName[256];	// ����̃t�@�C����
Extern char eTuneFpRegFileName[256];	// �`���[�j���O���[�h�ł̓o�^�摜��
Extern char eConvMovieFileName[256];	// �ϊ��擮��̃t�@�C����
Extern char eCamHarrisRegFileName[256];	// Harris�ɂ������_���o���[�h�ł̓o�^�摜��
Extern int eDetectHarrisCamMode;	// DetectHarrisCam�̃��[�h

Extern int eIsJp;	// ���{�ꃂ�[�h���ǂ���
Extern int eJpAdpBlock;
Extern int eJpAdpSub;
Extern int eJpGParam;
Extern int eJpThr;

// �ȉ��Cnn5dcam�p
#ifndef	kMaxPathLen
#define	kMaxPathLen	(128)	/* �p�X�̕�����̍ő咷 */
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
Extern char eThumbDir[kMaxPathLen];	// �T���l�C���̃f�B���N�g��
Extern char eThumbSuffix[kMaxPathLen];	// �T���l�C���̊g���q
Extern double eThumbScale;	// �T���l�C���̔{��

Extern char eHarrisTestOrigFileName[kMaxPathLen];
Extern char eHarrisTestAnnoFileName[kMaxPathLen];

Extern char eCopyright[kMaxLineLen];	// �R�s�[���C�g
