/* �O���[�o���ϐ���1�ӏ��ɏW�߂邽�߂̒�` */
#ifdef	GLOBAL_DEFINE 
	#define	Extern    /*define*/
#else
	#define	Extern    extern
#endif

Extern int eNumCom1;	// nCm
Extern int eNumCom2;	// mC(f or f-1)
Extern int eGroup1Num;	// �p�����[�^n
Extern int eGroup2Num;	// �p�����[�^m
Extern int eGroup3Num;	// f or f-1
Extern int eDiscNum;	// ���U�����x��
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
Extern int eCPF2StartNum;	// �����_���o���[�h�Q�ł̊J�n�ԍ�
Extern int eIsJp;	// ���{�ꃂ�[�h���ǂ���
Extern int eExperimentMode;	// �������[�h�i�n�b�V���T�C�Y�E�������ԁE���x�̏o�́j
Extern int eCompressHash;	// �n�b�V�����k���[�h

// ini�t�@�C���֌W
Extern int eTCPPort;	// TCP�ʐM�|�[�g
Extern int eProtocol;	// �ʐM�v���g�R��
Extern int ePointPort;	// �����_�|�[�g�iUDP�j
Extern int eResultPort;	// �������ʃ|�[�g�iUDP�j
Extern char eClientName[20];	// �N���C�A���g�̃}�V����
Extern char eServerName[20];	// �T�[�o�̃}�V����

// �����_���o�E���{�ꃂ�[�h
Extern int eJpGParam1;
Extern int eJpAdpBlock;
Extern int eJpAdpSub;
Extern int eJpGParam2;
Extern int eJpThr;

// �ł��؂�֌W
Extern int eTerminate;	// �ł��؂���@
Extern int eTermVNum;	// ���[���ł�臒l

// ���J�Ɍ����ĉ���
Extern char eHashSrcDir[kMaxPathLen];	// �n�b�V���̌��摜�t�@�C���̃f�B���N�g��
Extern char eHashSrcSuffix[kMaxPathLen];	// �n�b�V���̌��摜�t�@�C���̊g���q
Extern char ePointDatFileName[kMaxPathLen];	// point.dat�̃t�@�C����
Extern char eHashDatFileName[kMaxPathLen];	// hash.dat�̃t�@�C����
Extern int eEntireMode;	// �S�̂̃��[�h
Extern int eHashGaussMaskSize;	// �d�q�����̓����_���o�̃K�E�V�A���t�B���^�̃T�C�Y
Extern int eConnectHashImageThr;	// �d�q�����̓����_���o��2�l����臒l
Extern int eDocNumForMakeDisc;	// ���U���t�@�C���̍쐬�ɗp��������_�t�@�C���̐�
Extern int ePropMakeNum;	// ���萔�𒲂ׂ�ۂɗp���镶����
Extern char eConfigFileName[kMaxPathLen];	// config.dat�̃t�@�C����
Extern char eCopyright_2006_TomohiroNakai_IMP_OPU[kMaxLineLen];	// �R�s�[���C�g

// inacr2�p
Extern double eDiscMin;
Extern double eDiscMax;

// �n�b�V�����k���[�h�֌W
Extern int eDocBit;	// doc�̃r�b�g��
Extern int ePointBit;	// point�̃r�b�g��
Extern int eRBit;	// �s�ϗ�1�̃r�b�g��
Extern int eOBit;	// �ʐϓ�����1�̃r�b�g��
Extern int eHList2DatByte;	// ���k���X�g�̃f�[�^���̃o�C�g��

// ��]�̑���������s��Ȃ�
Extern int eRotateOnce;

// �Փ˂̑������̂��n�b�V�����珜��
Extern int eRemoveCollision;
Extern int eMaxHashCollision;

// �����ʂ̔�r���s��Ȃ�
Extern int eNoCompareInv;

// �n�b�V���e�[�u���ɒ��ڋL�^����i�����ʂ��Ȃ��j
Extern int eNoHashList;

// �����ꃂ�[�h
Extern int eVariousAutoMaskSize;
Extern int eVariousAutoMaskSizeDual;

// �f���A���n�b�V�����[�h�i���g�p�H�j
Extern int eDualHashMode;

// �����n�b�V�����[�h���̃n�b�V���̐��i���g�p�j
Extern int eHashNum;
