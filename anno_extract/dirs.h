#define	kNears	30	/* CR�̏ꍇ */
#define	kRadRange (M_PI / 4.0)
#define	kRightRange kRadRange
#define	kLeftRange kRadRange
#define	kUpRange kRadRange
#define	kDownRange kRadRange
#define	kMaxDiscriptorSize	(4096)
#define	kWordSep	(1.4)
#define	kFFRAngle	(M_PI / 4.0)		/* FirstFirstRightPointCR�p�u�������������v*/
#define	kRoughlyAngle	(M_PI / 20.0)	/* FirstRightPointCR�p�u��⓯���v*/
#define	kNearlyAngle	(M_PI / 18.0)	/* RightPointCR�p�u�قƂ�Ǔ����v*/
#define	kNLAngle	(M_PI / 4.0)	/* NextLineCR�p */
//#define	kCloseAngle	(M_PI / 14400.0)	/* RightPointCR�p�@�s���̕����Ƃ̊p�x*/
#define	kCloseAngle	((long double)(M_PI / 1000000000.0L))	/* RightPointCR�p�@�s���̕����Ƃ̊p�x */
#define	kAlmostAngle	(M_PI / 4.0)	/* RightPointCR�p�@�E���ɂȂ��ƍ���̂�*/

#define	kLittleVal	(0.000001L)	/* CalcCR�p */

#define	cWhite	cv::Scalar(255,255,255)
#define	cBlack	cv::Scalar(0,0,0)
#define	cRed	cv::Scalar(0,0,255)
#define	cGreen	cv::Scalar(0,255,0)
#define	cBlue	cv::Scalar(255,0,0)
#define	cRandom	cv::Scalar(rand(,rand()%256,rand()%256)%256 )

// �S�̂̃��[�h
#define	RETRIEVE_MODE	(0)
#define	CONST_HASH_MODE	(1)
#define	RET_MP_MODE			(2)
#define	TEST_MODE		(3)
#define	ADD_HASH_MODE	(4)
#define	CREATE_QPF_MODE	(5)	/* Create point file of query image */
#define	CHK_HASH_MODE	(6)	/* Check and analyse the hash table */
#define	CREATE_RPF_MODE	(7)	/* Create point file of registered image */
// �ˉe�ϊ��̕␳�����邩���Ȃ���
#define	NOT_RECOVER_MODE	(0)
#define	RECOVER_MODE		(1)
// �����_�͉����g����
#define	CONNECTED_MODE	(0)
#define	ENCLOSED_MODE	(1)
#define	USEPF_MODE		(2)	/* kPFPrefix, kPFSuffix�̓_�t�@�C�����g�� */
// �����摜�͗p�ӂ���Ă��邩�i�c�����j
#define	NOT_PREPARED_MODE	(0)
#define	PREPARED_MODE		(1)
#define	LEAVE_MODE			(2)

// �s�ϗʃ^�C�v
#define	CR_AREA		(0)
#define	CR_INTER	(1)
#define	AFFINE		(2)
#define	SIMILAR		(3)

#define	GetPointsDistance(p1, p2)	(sqrt((p1.x - p2.x)*(p1.x - p2.x)+(p1.y - p2.y)*(p1.y - p2.y)))

