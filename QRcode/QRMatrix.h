/*
�p�q�쐬�菇
�P�FCQRMatrix�̃C���X�^���X���쐬����
��j
CQRMatrix qrArray;

�Q�FQR�ŕ\������f�[�^��o�^����
��j
qrArray.AddQRAscii( "http://www.spoonsoftware.com" );

�R�F���������x�����w�肵��QR�R�[�h���쐬����
��j
qrArray.MakeQRArray( COLLECTLEVEL_M );

�S�FGetModuleNumber���Ăяo���A�}�g���b�N�X�̍s��ԍ����擾����
��j
int iModuleNumber = qrArray.GetModuleNumber();

�T�F�Q�����̃��[�v���s���A�c���f�[�^�ɃA�N�Z�X����
��j
for( int i=0 ; i<iModuleNumber ; i++ )
{
	for( int j=0 ; j<iModuleNumber ; j++ )
	{
		if( qrArray.GetModulePoint( j , i ) )
		{
			printf( "��" );
		}
		else
		{
			printf( "�@" );
		}
	}
}
*/

#ifndef _QRARRAY_H_
#define	_QRARRAY_H_

#include <list>
#include "QRDataElement.h"

using namespace std;

//���������x��
#define	COLLECTLEVEL_L		(0x01)
#define	COLLECTLEVEL_M		(0x00)
#define	COLLECTLEVEL_Q		(0x03)
#define	COLLECTLEVEL_H		(0x02)

//QR�쐬���A�G���[�ԍ�
#define	QRERR_NOERROR				(0)
#define	QRERR_WRONG_COLLECTTYPE		(1)
#define QRERR_DATASIZE_LARGE		(2)
#define	QRERR_DONTSET_TYPENUMBER	(3)
#define	QRERR_DATAISNOT_NUMERIC		(4)
#define	QRERR_DATAISNOT_ALPHA		(5)
#define	QRERR_DATAISNOT_KANJI		(6)
#define	QRERR_WRONGDIRECT_BIT		(7)
#define	QRERR_DATABITCOUNT_FAILURE	(8)

//QR�R�[�h�̊e�p�^�[��
#define	QR_UNKNOWN				(-1)
#define QR_DATA_LIGHT			(0)
#define QR_DATA_DARK			(1)
#define	QR_COLLECT_LIGHT		(2)
#define	QR_COLLECT_DARK			(3)
#define	QR_POSITIONDITECT_LIGHT	(10)
#define	QR_POSITIONDITECT_DARK	(11)
#define	QR_POSITIONCHECK_LIGHT	(12)
#define	QR_POSITIONCHECK_DARK	(13)
#define	QR_TIMING_LIGHT			(14)
#define	QR_TIMING_DARK			(15)
#define	QR_TYPE_LIGHT			(20)
#define	QR_TYPE_DARK			(21)
#define	QR_MODEL_LIGHT			(22)
#define	QR_MODEL_DARK			(23)

#define	QR_MASK_LIGHT			(50)
#define	QR_MASK_DARK			(51)

class CQRMatrix
{
public:
	CQRMatrix();
	~CQRMatrix();

	//�f�[�^�Z�b�g�n
	bool AddQRCharacters( const char * pcszString , int iCharacterType );

	bool MakeQRMatrix( int iCollectLevel , int & iErrorType );

	///////////////////////////////////////////////////////////
	///		QR�R�[�h�̌^�����擾����
	///		�^����1�`40�ƂȂ�
	///////////////////////////////////////////////////////////
	inline
	int GetTypeNumber()
	{
		return m_iModelTypeIndex;
	}

	///////////////////////////////////////////////////////////
	///		���W���[�������擾����
	///		���W���[�������}�g���b�N�X�̏c�E�����ƂȂ�
	///////////////////////////////////////////////////////////
	inline 
	int GetModuleNumber()
	{
		return m_iModuleNumber;
	}

	///////////////////////////////////////////////////////////
	///		�w�肳�ꂽ�s��ɑΉ�����|�C���g�f�[�^���擾����
	///		�߂�l�F�@==false�F���̈�
	///				�@==true�F�×̈�
	///////////////////////////////////////////////////////////
	inline
	bool IsDarkPoint( int iColumnIndex , int iRowIndex )
	{
		if( iColumnIndex < 0 || iColumnIndex >= m_iModuleNumber ||
			iRowIndex    < 0 || iRowIndex    >= m_iModuleNumber )
		{
			return false;
		}

		int iModulePointOrg;

		//�������̈�̏ꍇ
		if( m_ppucQRMatrix[iColumnIndex][iRowIndex] < (char)0 )
		{
			iModulePointOrg = 0;
		}
		//�����ςݗ̈�̏ꍇ
		else
		{
			iModulePointOrg = GetModulePoint( iColumnIndex , iRowIndex ) % 2;
		}

		//�}�X�N��������Ȃ��f�[�^�̏ꍇ
		if( m_ppucMaskMatrix[iColumnIndex][iRowIndex] == QR_UNKNOWN || m_iQRMaskPattern < 0 )
		{
			return (iModulePointOrg !=0 );
		}

		//�}�X�N��������f�[�^�̏ꍇ
		return (((GetMaskPoint( iColumnIndex , iRowIndex ) % 2) ^ iModulePointOrg) != 0 );
	}

protected:
	list<CQRDataElement> m_listQRData;
	void ReleaseAll();
	void ReleaseQRMatrix();
	void ReleaseMaskMatrix();
	void ReleaseData();
	void ReleaseError();

	bool GetDataBitNumber( int & iDataBit1_9 , int & iDataBit10_26 , int & iDataBit27_40 );

	bool CreateDataBits( int & iErrorType );
	unsigned char GetBitMask( int iBitNumber );
	bool SetTypeNumber(int iCollectLevel , int & iErrorType );
	bool InitModuleArea(int & iErrorType);
	bool SetPositionDitectPattern( int & iErrorType );
	bool SetPositionCheckPattern( int & iErrorType );
	bool SetTimingPattern( int & iErrorType );

	bool SetTypeInformation( bool bReserveOnly , int & iErrorType );
	bool SetModelInformation( int & iErrorType );

	bool CreateMaskPatternArea( int & iErrorType );

	bool AddDataBits( unsigned char ucData , int iBitNumber , int & iErrorType );
	bool AddDataBits( unsigned short usData , int & iErrorType );
	bool AddDataBits_Numeric( const char * pcszData , int iDataLength , int & iErrorType );
	bool AddDataBits_Alphabet( const char * pcszData , int iDataLength , int & iErrorType );
	bool AddDataBits_Ascii( const char * pcszData , int iDataLength , int & iErrorType );
	bool AddDataBits_Kanji( const char * pcszData , int iDataLength , int & iErrorType );

	bool OutputDataBits( int & iErrorType );
	unsigned char GetDataBit( int iBitIndex , bool bDataRequest );
	bool GetNextDataPutPoint( int & iColumnIndex , int & iRowIndex
									, bool & bFirstColumn
									, bool & bDirectionUp );

	bool DecideMaskPattern( int & iErrorType );
	int GetLostPoint_SeqModule( int iSeqBlock );
	int GetLostPoint_ModuleBlock( int iColumnIndex , int iRowIndex );
	int GetLostPoint_PositionDitect( int iColumnIndex , int iRowIndex );
	int GetLostPoint_BlackWhiteRatio( int iBlackPoint );

	bool CreateErrorCollectData( int & iErrorType );
	bool SeparateTotalNumber( int iSeparateCount , int iTotalNumber
							 , int & iFirstCount , int & iFirstNumber
							 , int & iSecondCount , int & iSecondNumber );

	///////////////////////////////////////////////////////////
	///		�w�肳�ꂽ�s��ɑΉ�����|�C���g�f�[�^���Z�b�g����
	///////////////////////////////////////////////////////////
	inline
	bool SetModulePoint( int iColumnIndex , int iRowIndex , int iQRPoint )
	{
		if( iColumnIndex < 0 || iColumnIndex >= m_iModuleNumber ||
			iRowIndex    < 0 || iRowIndex    >= m_iModuleNumber )
		{
			return false;
		}
		m_ppucQRMatrix[iColumnIndex][iRowIndex] = iQRPoint;

		return true;
	}

	///////////////////////////////////////////////////////////
	///		�w�肳�ꂽ�s��ɑΉ�����|�C���g�f�[�^���擾����
	///////////////////////////////////////////////////////////
	inline 
	int GetModulePoint( int iColumnIndex , int iRowIndex )
	{
		if( iColumnIndex < 0 || iColumnIndex >= m_iModuleNumber ||
			iRowIndex    < 0 || iRowIndex    >= m_iModuleNumber )
		{
			return QR_UNKNOWN;
		}

		return m_ppucQRMatrix[iColumnIndex][iRowIndex];
	}

	///////////////////////////////////////////////////////////
	///		�w��s�A��̃}�X�N�p�^�[�����擾����
	///////////////////////////////////////////////////////////
	inline 
	int GetMaskPoint( int iColumnIndex , int iRowIndex , int iQRMaskPattern = -1)
	{
		if( iQRMaskPattern < 0 )
		{
			iQRMaskPattern = m_iQRMaskPattern;
		}

		if( iQRMaskPattern < 0 || iQRMaskPattern > 7 )
		{
			return QR_UNKNOWN;
		}

		bool bMask;

		//�d�l����i,j�Ȃ̂ŁA�������Ȃ��悤�ɕϊ�
		int i = iRowIndex;
		int j = iColumnIndex;

		switch( iQRMaskPattern )
		{
		case 0:	bMask = ((i + j) % 2 == 0);							break;
		case 1:	bMask = (i % 2 == 0);								break;
		case 2:	bMask = (j % 3 == 0);								break;
		case 3:	bMask = ((i + j) % 3 == 0);							break;
		case 4:	bMask = (((i / 2) + (j / 3)) % 2 == 0 );			break;
		case 5:	bMask = ((i * j) % 2 + (i * j) % 3 == 0);			break;
		case 6:	bMask = (((i * j) % 2 + (i * j) % 3 ) % 2 == 0);	break;
		case 7:	bMask = (((i * j) % 3 + (i + j) % 2 ) % 2 == 0);	break;
		}

		return (bMask)?(QR_MASK_DARK):(QR_MASK_LIGHT);
	}

	bool GetExpressionMod( unsigned char *puszBaseData , int iBaseDataLength,
						   unsigned char *puszCoefficients ,
						   unsigned char *puszError , int iErrorLength );


protected:

	//QR�f�[�^�̃}�g���b�N�X�{��
	//�e�v�f�͈ȉ��̂Ƃ���ƂȂ�B
	//�@< 0 �F�������̈�(QR_UNKNOWN)
	//  % 2 == 0 �F���̈�
	//  % 2 != 0 �F�×̈�
	//
	//�@== 0 �F�f�[�^�̈�i���j(QR_DATA_LIGHT)
	//  == 1 �F�f�[�^�̈�i�Áj(QR_DATA_DARK)
	//  == 2 �F�G���[�����̈�i���j(QR_COLLECT_LIGHT)
	//  == 3 �F�G���[�����̈�i�Áj(QR_COLLECT_DARK)
	//  == 10�F�ʒu���o�p�^�[���i���j�i�����p�^�[�����܂ށj(QR_POSITIONDITECT_LIGHT)
	//  == 11�F�ʒu���o�p�^�[���i�Áj�i�����p�^�[�����܂ށj(QR_POSITIONDITECT_DARK)
	//  == 12�F�ʒu���킹�p�^�[���i���j(QR_POSITIONCHECK_LIGHT)
	//  == 13�F�ʒu���킹�p�^�[���i�Áj(QR_POSITIONCHECK_DARK)
	//  == 14�F�^�C�~���O�p�^�[���i���j(QR_TIMING_LIGHT)
	//  == 15�F�^�C�~���O�p�^�[���i�Áj(QR_TIMING_DARK)
	//  == 20�F�`�����i���j(QR_TYPE_LIGHT)
	//  == 21�F�`�����i�Áj(QR_TYPE_DARK)
	//  == 22�F�^�ԏ��i���j(QR_MODEL_LIGHT)
	//  == 23�F�^�ԏ��i�Áj(QR_MODEL_DARK)
	char ** m_ppucQRMatrix;

	char ** m_ppucMaskMatrix;

	int m_iModuleNumber;
	
	int m_iModelTypeIndex;

	int m_iDataBitNumber;
	int m_iDataSizeNumber;
	unsigned char * m_pucData;

	int m_iErrorSizeNumber;
	unsigned char * m_pucError;

	int m_iQRMaskPattern;
};

#endif

