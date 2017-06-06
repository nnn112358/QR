
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "QRMatrix.h"

#include "QRInformation.h"
#include "QRReadSolomon.h"

///////////////////////////////////////////////////////////////////////////////
///		CQRMatrix�̒�`
///////////////////////////////////////////////////////////////////////////////
CQRMatrix::CQRMatrix()
{
	m_iModelTypeIndex = -1;
	m_iModuleNumber = 0;
	m_ppucQRMatrix = NULL;
	m_ppucMaskMatrix = NULL;

	m_iDataBitNumber = 0;
	m_iDataSizeNumber = 0;
	m_pucData = NULL;

	m_iErrorSizeNumber = 0;
	m_pucError = NULL;

	m_iQRMaskPattern = -1;
}

CQRMatrix::~CQRMatrix()
{
	ReleaseAll();
}

///////////////////////////////////////////////////////////
///		�������X�g�ɕ�����f�[�^���Z�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::AddQRCharacters( const char * pcszString , int iCharacterType )
{
	CQRDataElement qrElement;

	bool bDataAdded = false;

	//�P�ȏ�̃f�[�^���o�^����Ă���ꍇ
	if( m_listQRData.begin() != m_listQRData.end() )
	{
		list<CQRDataElement>::iterator pQRElement = m_listQRData.end();
		pQRElement--;

		//�Ō�̃f�[�^���[�h�ƌ��݂̃f�[�^���[�h����v���Ă���ꍇ�ɂ͍Ō�̃��X�g��
		//�������ǉ�����
		bDataAdded = pQRElement->AddData( pcszString , (int)strlen(pcszString) , iCharacterType );
	}

	if( !bDataAdded )
	{
		qrElement.SetData( pcszString , (int)strlen( pcszString ) , iCharacterType );
		m_listQRData.push_back( qrElement );
	}
	

	return true;
}

///////////////////////////////////////////////////////////
///		�S�f�[�^���폜����
///////////////////////////////////////////////////////////
void CQRMatrix::ReleaseAll()
{
	ReleaseQRMatrix();
	ReleaseMaskMatrix();
	ReleaseData();
	ReleaseError();

	m_iModuleNumber = 0;
	m_iModelTypeIndex = -1;
}

///////////////////////////////////////////////////////////
///		QR�̃f�[�^���폜����
///////////////////////////////////////////////////////////
void CQRMatrix::ReleaseQRMatrix()
{
	if( m_ppucQRMatrix != NULL )
	{
		for( int i=0 ; i<m_iModuleNumber ; i++ )
		{
			delete[] m_ppucQRMatrix[i];
		}
		delete[] m_ppucQRMatrix;
	}
	m_ppucQRMatrix = NULL;
}

///////////////////////////////////////////////////////////
///		�}�X�N�̃f�[�^���폜����
///////////////////////////////////////////////////////////
void CQRMatrix::ReleaseMaskMatrix()
{
	if( m_ppucMaskMatrix )
	{
		for( int i=0 ; i<m_iModuleNumber ; i++ )
		{
			delete[] m_ppucMaskMatrix[i];
		}
		delete[] m_ppucMaskMatrix;
	}

	m_ppucMaskMatrix = NULL;
}

void CQRMatrix::ReleaseData()
{
	if( m_pucData )
	{
		delete[] m_pucData;
	}
	m_pucData = NULL;
	m_iDataSizeNumber = 0;
}

void CQRMatrix::ReleaseError()
{
	if( m_pucError )
	{
		delete[] m_pucError;
	}
	m_pucError = NULL;
	m_iErrorSizeNumber = 0;
}

///////////////////////////////////////////////////////////
///		�Z�b�g����Ă���f�[�^�����ɂp�q�}�g���b�N�X���쐬����
///		�����F
///			iCollectLevel	�F	���������x��
///									COLLECTLEVEL_L
///									COLLECTLEVEL_M
///									COLLECTLEVEL_Q
///									COLLECTLEVEL_H
///								�̂����ꂩ���w�肷��
///		�߂�l�F
///			==false�F�쐬�Ɏ��s�i�f�[�^���傫������Ȃǁj
///			==true�F�쐬�ɐ���
///////////////////////////////////////////////////////////
bool CQRMatrix::MakeQRMatrix( int iCollectLevel , int & iErrorType )
{
	//���ݐݒ肳��Ă���QR�f�[�^�̈���폜����
	ReleaseAll();

	bool bRet = true;
	iErrorType = QRERR_NOERROR;

	//�^�ԏ���ݒ肷��
	bRet = SetTypeNumber( iCollectLevel , iErrorType );

	//�f�[�^�r�b�g����쐬����
	if( bRet )
	{
		bRet = CreateDataBits( iErrorType );
	}

	//�������R�[�h������̍쐬
	if( bRet )
	{
		bRet = CreateErrorCollectData( iErrorType );
	}

	//�Z�b�g���ꂽ�^�ԍ�����A���W���[���z�������������
	if( bRet )
	{
		bRet = InitModuleArea( iErrorType );
	}

	//�ʒu���o�p�^�[���̃Z�b�g
	if( bRet )
	{
		bRet = SetPositionDitectPattern( iErrorType );
	}

	//�ʒu���킹�p�^�[���̃Z�b�g
	if( bRet )
	{
		bRet = SetPositionCheckPattern( iErrorType );
	}

	//�^�C�~���O�p�^�[���̃Z�b�g
	if( bRet )
	{
		bRet = SetTimingPattern( iErrorType );
	}

	//�`�����̉��o��
	if( bRet )
	{
		bRet = SetTypeInformation( true , iErrorType );
	}

	//�^�ԏ��
	if( bRet )
	{
		bRet = SetModelInformation( iErrorType );
	}

	//�}�X�N�p�^�[���K���̈���擾����
	if( bRet )
	{
		bRet = CreateMaskPatternArea( iErrorType );
	}

	//�f�[�^�r�b�g�̏o��
	if( bRet )
	{
		bRet = OutputDataBits( iErrorType );
	}

	//�}�X�N�p�^�[���̌���
	if( bRet )
	{
		bRet = DecideMaskPattern( iErrorType );
	}

	//�`�����̏o��
	if( bRet )
	{
		bRet = SetTypeInformation( false, iErrorType );
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�o�^����Ă���f�[�^����f�[�^��������쐬����
///////////////////////////////////////////////////////////
bool CQRMatrix::CreateDataBits( int & iErrorType )
{
	bool bRet = true;

	//�f�[�^�̃����[�X
	ReleaseData();

	list<CQRDataElement>::iterator pDataElement;
	for( pDataElement = m_listQRData.begin()
		;pDataElement != m_listQRData.end() && bRet 
		;pDataElement ++ )
	{
		//�f�[�^���[�h�̏o��
		unsigned char cModel = 0x00;
		switch( pDataElement->m_iDataMode )
		{
		case QRCHARACTERTYPE_NUMERIC:
			bRet = AddDataBits_Numeric( pDataElement->m_pszData , pDataElement->m_iDataLength , iErrorType);
			break;

		case QRCHARACTERTYPE_ALPHABET:
			bRet = AddDataBits_Alphabet( pDataElement->m_pszData , pDataElement->m_iDataLength , iErrorType );
			break;

		case QRCHARACTERTYPE_ASCII:
			bRet = AddDataBits_Ascii( pDataElement->m_pszData , pDataElement->m_iDataLength , iErrorType );
			break;

		case QRCHARACTERTYPE_KANJI:
			bRet = AddDataBits_Kanji( pDataElement->m_pszData , pDataElement->m_iDataLength , iErrorType );
			break;
		}

	}


	//�I�[�o��
	if( bRet )
	{
		bRet = AddDataBits( (unsigned char)0x00 , 4 , iErrorType);
	}

	if( bRet )
	{
		//�f�[�^�o�C�g�I�[�܂ł�0�Ŗ��߂�
		for( ; m_iDataBitNumber < (c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber &&
			   m_iDataBitNumber % 8 != 0 && bRet
			 ; )
		{
			bRet = AddDataBits( (unsigned char)0x00 , 1 , iErrorType );
		}

		bool bFirstByte = true;
		for( ; m_iDataBitNumber < (c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber && bRet
			 ;  )
		{
			if( bFirstByte )
			{
				bRet = AddDataBits( (unsigned char)0xec , 8 , iErrorType );
			}
			else
			{
				bRet = AddDataBits( (unsigned char)0x11 , 8 , iErrorType );
			}
			bFirstByte = !bFirstByte;
		}
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�o�^�ς݂̃f�[�^���X�g����f�[�^�̃r�b�g�����擾����
///		�p�q�̌^�Ԃɂ��A�T�C�Y���قȂ邽�߁A�e�^�Ԏ��̃f�[�^�T�C�Y��
///		�Z�b�g����
///		�����F
///			iDataBit1_9		(OUT)	1�^�`9�^�̃f�[�^�r�b�g��
///			iDataBit10_26	(OUT)	10�^�`26�^�̃f�[�^�r�b�g��
///			iDataBit27_40	(OUT)	27�^�`40�^�̃f�[�^�r�b�g��
///////////////////////////////////////////////////////////
bool CQRMatrix::GetDataBitNumber( int & iDataBit1_9 , int & iDataBit10_26 , int & iDataBit27_40 )
{
	bool bRet = true;

	//������
	iDataBit1_9 = iDataBit10_26 = iDataBit27_40 = 0;
	//�^�Ɋ֌W�Ȃ��f�[�^�r�b�g��
	int iDataBits = 0;

	list<CQRDataElement>::iterator pDataElement;
	for( pDataElement = m_listQRData.begin()
		; pDataElement != m_listQRData.end()
		; pDataElement++ )
	{
		//�f�[�^�w���q
		iDataBits += 4;

		switch( pDataElement->m_iDataMode )
		{
		//�������[�h�̏ꍇ
		case QRCHARACTERTYPE_NUMERIC:

			//�f�[�^�T�C�Y�o�͗p
			iDataBit1_9   += 10;
			iDataBit10_26 += 12;
			iDataBit27_40 += 14;

			//�f�[�^�{�̏o�͗p
			iDataBits += ( pDataElement->m_iDataLength / 3 ) * 10;
			if( pDataElement->m_iDataLength % 3 == 1 )
			{
				iDataBits += 4;
			}
			else if( pDataElement->m_iDataLength % 3 == 2 )
			{
				iDataBits += 7;
			}
			break;

		//�A���t�@�x�b�g���[�h�̏ꍇ
		case QRCHARACTERTYPE_ALPHABET:

			//�f�[�^�T�C�Y�o�͗p
			iDataBit1_9   += 9;
			iDataBit10_26 += 11;
			iDataBit27_40 += 13;

			//�f�[�^�{�̏o�͗p
			iDataBits += ( pDataElement->m_iDataLength / 2 ) * 11;
			if( pDataElement->m_iDataLength % 2 ) 
			{
				iDataBits += 6;
			}

			break;

		//ASCII������
		case QRCHARACTERTYPE_ASCII:

			//�f�[�^�T�C�Y�o�͗p
			iDataBit1_9 += 8;
			iDataBit10_26 += 16;
			iDataBit27_40 += 16;

			iDataBits += pDataElement->m_iDataLength * 8;
			break;

		//����������p
		case QRCHARACTERTYPE_KANJI:

			//�f�[�^�T�C�Y�o�͗p
			iDataBit1_9   += 8;
			iDataBit10_26 += 10;
			iDataBit27_40 += 12;

			//�f�[�^�o�͗p
			iDataBits += (pDataElement->m_iDataLength / 2) * 13;

			break;

		default:
			return false;
		}
	}

	//�I�[���[�h�w���q
	iDataBits += 4;

	iDataBit1_9 += iDataBits;
	iDataBit10_26 += iDataBits;
	iDataBit27_40 += iDataBits;

	return bRet;
}

///////////////////////////////////////////////////////////
///		�����f�[�^�̃Z�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Numeric( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//�f�[�^�������݂̂��ǂ����̃`�F�b�N
	for( int i=0 ; i<iDataLength ; i++ )
	{
		char cTemp = pcszData[i];

		//�����łȂ��ꍇ�͑��߂�
		if( !isdigit( cTemp ) )
		{
			iErrorType = QRERR_DATAISNOT_NUMERIC;
			return false;
		}

	}

	//�����̎w���q�o��
	bRet = AddDataBits( (unsigned char)0x01 , 4 , iErrorType );

	//�������w���q�̏o��
	short sOutputNumber = (short)iDataLength;

	int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;

	//�P�`�X�^�̏ꍇ�i�P�O�r�b�g�j
	if( iModelNumber <= 9 )
	{
		bRet = AddDataBits( (unsigned char)( (sOutputNumber >> 8) & 0x03 ) , 2 , iErrorType );
	}

	//�P�O�`�Q�U�^�̏ꍇ�i�P�Q�r�b�g�j
	else if(iModelNumber <= 26 )
	{
		bRet = AddDataBits( (unsigned char)( (sOutputNumber >> 8) & 0x0f ) , 4 , iErrorType );
	}

	//�Q�V�`�S�O�^�̏ꍇ�i�P�S�r�b�g�j
	else
	{
		bRet = AddDataBits( (unsigned char)( (sOutputNumber >> 8) & 0x3f ) , 6 , iErrorType );
	}

	//���ʂW�r�b�g�o��
	if( bRet )
	{
		bRet = AddDataBits( (unsigned char)( sOutputNumber & 0xff ) , 8 , iErrorType );
	}

	for( int i=0 ; i<iDataLength && bRet ; i+= 3 )
	{
		char szTemp[4];
		int iIndex = 0;

		for( int j=i ; j<iDataLength && iIndex < 3 ; j++,iIndex++ )
		{
			szTemp[iIndex] = pcszData[j];
		}
		szTemp[iIndex] = '\0';

		short sTemp = (short)atoi( szTemp );
		size_t sizeLength = strlen( szTemp );

		//�P�����̂�
		if( sizeLength == 1 )
		{
			bRet = AddDataBits( (unsigned char)(sTemp & 0x0f) , 4 , iErrorType );
		}
		//�Q����
		else if( sizeLength == 2 )
		{
			bRet = AddDataBits( (unsigned char)(sTemp & 0x7f) , 7 , iErrorType );
		}
		//�R����
		else if( sizeLength == 3 )
		{
			bRet = AddDataBits( (unsigned char)((sTemp >> 8) & 0x03) , 2 , iErrorType );
			if( bRet )
			{
				bRet = AddDataBits( (unsigned char)( sTemp & 0xff ) , 8 , iErrorType );
			}
		}
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�A���t�@�x�b�g�f�[�^�̃Z�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Alphabet( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//�f�[�^���p�����݂̂��ǂ������`�F�b�N����
	for( int i=0 ; i<iDataLength ; i++ )
	{
		char cTemp = pcszData[i];
		bRet = false;
		for( int j = 0 ; cszQrAlphabets[j] ; j++ )
		{
			if( cTemp == cszQrAlphabets[j] )
			{
				bRet = true;
				break;
			}
		}

		if( !bRet )
		{
			return false;
		}
	}

	//�A���t�@�x�b�g�̎w���q�o��
	bRet = AddDataBits( (unsigned char)0x02 , 4 , iErrorType );

	//�������̏o��
	unsigned short usLength = (unsigned short)iDataLength;
	int iModelTypeNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;
	
	//�P�`�X�^
	if( iModelTypeNumber <= 9 )
	{
		bRet = AddDataBits( (unsigned char)(( usLength >> 8 ) & 0x01) , 1 , iErrorType );
	}
	//�P�O�`�Q�U�^
	else if( iModelTypeNumber <= 26 )
	{
		bRet = AddDataBits( (unsigned char)(( usLength >> 8 ) & 0x07) , 3 , iErrorType );
	}
	//�Q�V�`�S�O�^
	else
	{
		bRet = AddDataBits( (unsigned char)(( usLength >> 8 ) & 0x1f) , 5 , iErrorType );
	}

	if( bRet )
	{
		bRet = AddDataBits( (unsigned char)(usLength & 0xff) , 8 , iErrorType );
	}

	//�Q�������P�P�r�b�g�ɕϊ�����
	for( int i=0 ; i<iDataLength && bRet ; i+=2 )
	{
		unsigned short usChar = 0;
		int iChar;

		for( iChar = 0; iChar < 2 && i+iChar<iDataLength ; iChar++ )
		{
			usChar *= 45;

			for( int j=0 ; cszQrAlphabets[j] ; j++ )
			{
				if( cszQrAlphabets[j] == pcszData[i+iChar] )
				{
					usChar += (unsigned char)j;
				}
			}
		}

		//�P�����݂̂̏o�͂̏ꍇ�i�Ubit�o�́j
		if( iChar == 1 )
		{
			bRet = AddDataBits( (unsigned char)(usChar & 0x3f) , 6 , iErrorType );
		}
		//�Q�����̏o�͂̏ꍇ�i�P�Pbit�o�́j
		else if( iChar == 2 )
		{
			bRet = AddDataBits( (unsigned char)((usChar >> 8) & 0x07) , 3 , iErrorType );
			if( bRet )
			{
				bRet = AddDataBits( (unsigned char)(usChar & 0xff) , 8 , iErrorType );
			}
		}
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�A�X�L�[�f�[�^�̃Z�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Ascii( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//�A�X�L�[�̎w���q�o��
	bRet = AddDataBits( (unsigned char)0x04 , 4 , iErrorType );
	
	if( bRet )
	{
		//�f�[�^���̏o��
		int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;

		//�P�^�`�X�^
		if( iModelNumber <= 9 )
		{
			bRet = AddDataBits( (unsigned char)iDataLength , 8 , iErrorType );
		}
		//�P�O�^�`�S�O�^
		else
		{
			bRet = AddDataBits( (unsigned short)iDataLength , iErrorType );
		}
	}

	for( int i=0 ; i<iDataLength && bRet ; i++ )
	{
		unsigned char ucTemp = (unsigned char)pcszData[i];
		bRet = AddDataBits( ucTemp , 8 , iErrorType );
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�����f�[�^�̃Z�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Kanji( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//�������ǂ����̃`�F�b�N
	for( int i=0 ; i<iDataLength ; i+=2 )
	{
		unsigned short usTemp = (((((unsigned short)pcszData[i]) << 8) & 0xff00) |
									((unsigned short)pcszData[i+1] & 0x00ff) );

		//0x8140�`0x9ffc
		if( !(0x8140 <= usTemp && usTemp <= 0x9ffc || 
			  0xe040 <= usTemp && usTemp <= 0xebbf) )
		{
			iErrorType = QRERR_DATAISNOT_KANJI;
			return false;
		}

	}

	//�������[�h
	bRet = AddDataBits( (unsigned char)0x08 , 4 , iErrorType );
	if( !bRet )
	{
		return bRet;
	}

	//�������̏o��
	short sLength = (short)(iDataLength / 2);
	int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;

	//�P�`�X�^
	if( iModelNumber <= 9 )
	{
		bRet = AddDataBits( (unsigned char)( sLength & 0xff ) , 8 , iErrorType );
	}
	else
	{
		//�P�O�`�Q�U�^
		if( iModelNumber <= 26 )
		{
			bRet = AddDataBits( (unsigned char)((sLength >> 8) & 0x03) , 2 , iErrorType );
		}
		//�Q�V�`�S�O�^
		else
		{
			bRet = AddDataBits( (unsigned char)((sLength >> 8) & 0x0f) , 4 , iErrorType );
		}

		if( bRet )
		{
			bRet = AddDataBits( (unsigned char)(sLength & 0xff) , 8 , iErrorType );
		}
	}

	//�����̏o��
	for( int i=0 ; i<iDataLength && bRet ; i+=2 )
	{
		unsigned short usCharacter = (((((unsigned short)pcszData[i]) << 8) & 0xff00) |
										((unsigned short)pcszData[i+1] & 0x00ff) );

		//0x8140�`0x9ffc
		if( 0x8140 <= usCharacter && usCharacter <= 0x9ffc )
		{
			//0x8140������
			usCharacter -= 0x8140;
		}
		//0xe040�`0xebbf
		else
		{
			//0xc140������
			usCharacter -= 0xc140;
		}
		usCharacter = (((usCharacter >> 8) & 0xff) * 0xc0)	//��ʃo�C�g��C0���悶��
					+  ((usCharacter) & 0xff);				//���ʃo�C�g�𑫂�

		//�f�[�^�̏o�͏���
		bRet = AddDataBits( ((unsigned char)(usCharacter >> 8) & 0x1f) , 5 , iErrorType );
		if( bRet )
		{
			bRet = AddDataBits( (unsigned char)(usCharacter & 0xff) , 8 , iErrorType );
		}
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�f�[�^�Ƀr�b�g��ǉ�����
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits( unsigned char ucData , int iBitNumber , int & iErrorType )
{
	bool bRet = true;

	if( iBitNumber <= 0 || iBitNumber > 8 )
	{
		iErrorType = QRERR_WRONGDIRECT_BIT;
		return false;
	}

	int iBitLast = m_iDataBitNumber + iBitNumber;
	//�K�v�o�C�g�T�C�Y
	int iByteNow = (m_iDataBitNumber + 7) / 8;
	int iByteNeed = (iBitLast + 7) / 8;

	//�ǉ��̃f�[�^�̈悪�K�v�ȏꍇ
	if( iByteNeed > m_iDataSizeNumber )
	{
		//128�o�C�g
		m_iDataSizeNumber += 128;
		unsigned char * puszTemp = new unsigned char[ m_iDataSizeNumber ];
		memset( puszTemp , 0 , m_iDataSizeNumber );

		if( m_pucData )
		{
#ifdef WIN32
			memcpy_s( puszTemp , m_iDataSizeNumber , m_pucData , iByteNow );
#else
			memcpy( puszTemp , m_pucData , iByteNow );
#endif
			delete[] m_pucData;
		}

		m_pucData = puszTemp;
	}

	//������Z�b�g����f�[�^�̃}�X�N
	unsigned char ucSetMask = GetBitMask( iBitNumber - 1 );

	//�f�[�^�̒ǉ�����
	for( int i=0 ; i < iBitNumber ; i++ )
	{
		//bit��on�̏ꍇ
		if( ucSetMask & ucData )
		{
			int iBitToSet = m_iDataBitNumber + i;

			//�Z�b�g�ς݃f�[�^�̎��̃f�[�^�̃}�X�N
			unsigned char ucDataMask = GetBitMask( 7 - (iBitToSet % 8) );

			//�Y���f�[�^��bit��on�ɂ���
			int iByteNow = iBitToSet / 8;

			m_pucData[iByteNow] |= ucDataMask;
			
		}
		ucSetMask = (ucSetMask >> 1) & 0x7f;
	}
	
	m_iDataBitNumber += iBitNumber;

	return bRet;
}

///////////////////////////////////////////////////////////
///		�Q�o�C�g�f�[�^���o�͂���
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits( unsigned short usData , int & iErrorType )
{
	bool bRet = true;

	bRet = AddDataBits( (unsigned char)((usData >> 8 ) & 0xff) , 8 , iErrorType );
	if( bRet )
	{
		bRet = AddDataBits( (unsigned char)(usData & 0xff) , 8 , iErrorType );
	}
	return bRet;
}

///////////////////////////////////////////////////////////
///		�w�肳�ꂽ�r�b�g�ԍ��̃r�b�g�}�X�N���擾����
///////////////////////////////////////////////////////////
unsigned char CQRMatrix::GetBitMask( int iBitNumber )
{
	unsigned char ucMask = 0x01 << iBitNumber;

	return ucMask;
}

///////////////////////////////////////////////////////////
///		�f�[�^�̃o�C�g���ƌ��������x������
///		QR�̌^�i1�^�`40�^�j�����肷��
///////////////////////////////////////////////////////////
bool CQRMatrix::SetTypeNumber(int iCollectLevel , int & iErrorType )
{
	int iByteNum = 0;

	//�w�肳�ꂽ���������x�����s���̏ꍇ
	if( iCollectLevel != COLLECTLEVEL_L && iCollectLevel != COLLECTLEVEL_M &&
		iCollectLevel != COLLECTLEVEL_Q && iCollectLevel != COLLECTLEVEL_H )
	{
		iErrorType = QRERR_WRONG_COLLECTTYPE;
		return false;
	}

	bool bRet = false;

	int iDataBit1_9 , iDataBit10_26 , iDataBit27_40;
	if( !GetDataBitNumber( iDataBit1_9 , iDataBit10_26 , iDataBit27_40 ) )
	{
		iErrorType = QRERR_DATABITCOUNT_FAILURE;
		return false;
	}

	//�^�ԍ������\���̂��猟������
	for( int i=0 ; (c_typeMatrix+i)->m_iModelNumber > 0 ; i++ )
	{
		//�^�ɂ���ăT�C�Y���قȂ邽�߁A���݂̃`�F�b�N�^�ɍ��킹���r�b�g�T�C�Y���擾����
		int iDataBitNumber = iDataBit1_9;
		if( (c_typeMatrix + i)->m_iModelNumber >= 27 )
		{
			iDataBitNumber = iDataBit27_40;
		}
		else if( (c_typeMatrix + i)->m_iModelNumber >= 10 )
		{
			iDataBitNumber = iDataBit10_26;
		}

		if( (c_typeMatrix+i)->m_iCollectLevel == iCollectLevel &&
			(c_typeMatrix+i)->m_iDataBitNumber >= iDataBitNumber )
		{
			m_iModelTypeIndex = i;

			bRet = true;
			break;
		}
	}

	//�G���[�����������ꍇ
	if( !bRet )
	{
		iErrorType = QRERR_DATASIZE_LARGE;
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�������R�[�h��̍쐬����
///////////////////////////////////////////////////////////
bool CQRMatrix::CreateErrorCollectData( int & iErrorType )
{
	bool bRet = true;

	ReleaseError();

	//���f�[�^�R�[�h�ꐔ�i�r�b�g���j
	int iErrorBit = (c_typeMatrix + m_iModelTypeIndex)->m_iTotalBits 
				  - (c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber;
	m_iErrorSizeNumber = iErrorBit / 8;
	m_pucError = new unsigned char[m_iErrorSizeNumber];

	int iFirstCount , iSecondCount;
	int iFirstDataNumber , iSecondDataNumber;

	int iSeparateCount = (c_typeMatrix + m_iModelTypeIndex)->m_iRSBlockNumber;
	int iTotalNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber / 8;

	SeparateTotalNumber( iSeparateCount , iTotalNumber , iFirstCount , iFirstDataNumber , iSecondCount , iSecondDataNumber );

	int iDataIndex = 0;
	int iErrorIndex = 0;
	int iErrorNumber = ( (c_typeMatrix + m_iModelTypeIndex)->m_iTotalBits
						-(c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber) / 8 / iSeparateCount;

	//�e�u���b�N���ƂɃ��[�h�\�������ɂ�镄���������s����
	for( int i=0 ; i<iSeparateCount ; i++ )
	{
		int iDataNumber = iSecondDataNumber;
		if( i < iFirstCount )
		{
			iDataNumber = iFirstDataNumber;
		}

		CQRReadSolomon qrReadSolomon;

		qrReadSolomon.CodeTo( m_pucData + iDataIndex , iDataNumber , m_pucError + iErrorIndex , iErrorNumber );

		iDataIndex += iDataNumber;
		iErrorIndex += iErrorNumber;
	}
	return bRet;
}

///////////////////////////////////////////////////////////
///		�f�[�^�̕���
///		�[������������ꍇ�AiFirstCount��iSecondCount�ɕ�������
///		�����ŕ����ł���悤�ɒ�������
///		RS�u���b�N�ɑ΂���f�[�^�����v�Z����̂Ɏg�p����
///////////////////////////////////////////////////////////
bool CQRMatrix::SeparateTotalNumber( int iSeparateCount , int iTotalNumber
									, int & iFirstCount , int & iFirstNumber
									, int & iSecondCount , int & iSecondNumber )
{
	iFirstNumber = iTotalNumber / iSeparateCount;
	
	iSecondCount = iTotalNumber - iFirstNumber * iSeparateCount;
	if( iSecondCount == 0 )
	{
		iSecondNumber = 0;
	}
	else
	{
		iSecondNumber = iFirstNumber + 1;
	}

	iFirstCount = iSeparateCount - iSecondCount;

	return true;
}

///////////////////////////////////////////////////////////
///		�ݒ肳�ꂽ�^�ԍ����g�p���āA���W���[���̈��
///		�m�ۂ��A����������
///////////////////////////////////////////////////////////
bool CQRMatrix::InitModuleArea( int & iErrorType )
{
	//�^�ԍ����烂�W���[�������v�Z
	m_iModuleNumber = 17 + (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber * 4;

	//���W���[���f�[�^�̗̈�m�ۂƏ�����
	m_ppucQRMatrix = new char*[m_iModuleNumber];
	//�}�X�N�f�[�^�̗̈�m�ۂƏ�����
	m_ppucMaskMatrix = new char*[m_iModuleNumber];

	for( int i=0 ; i<m_iModuleNumber ; i++ )
	{
		m_ppucQRMatrix[i]   = new char[m_iModuleNumber];
		m_ppucMaskMatrix[i] = new char[m_iModuleNumber];

		for( int j=0 ; j<m_iModuleNumber ; j++ )
		{
			m_ppucQRMatrix[i][j] = (char)QR_UNKNOWN;
			m_ppucMaskMatrix[i][j] = (char)QR_UNKNOWN;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
///		�ʒu���o�p�^�[���̃Z�b�g
///		�ʒu���o�p�^�[���̕����p�^�[���������ɃZ�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::SetPositionDitectPattern( int & iErrorType )
{
	bool bRet = true;

	char cPatterns[9][10] 
				= {"000000000"
				  ,"011111110"
				  ,"010000010"
				  ,"010111010"
				  ,"010111010"
				  ,"010111010"
				  ,"010000010"
				  ,"011111110"
				  ,"000000000"};

	//�ꌟ�o�p�^�[���ʒu
	int iStartPosX[3] = { -1 , m_iModuleNumber - 8 , -1 };
	int iStartPosY[3] = { -1 , -1                  , m_iModuleNumber - 8 };

	for( int i=0 ; i<3 ; i++ )
	{
		//�����p�^�[���̏o��
		for( int j=0 ; j<9 ; j++ )
		{
			for( int k=0 ; k<9 ; k++ )
			{
				int iModuleType = QR_POSITIONDITECT_LIGHT;

				if( cPatterns[j][k] == '1' )
				{
					iModuleType = QR_POSITIONDITECT_DARK;
				}

				int x = iStartPosX[i] + j;
				int y = iStartPosY[i] + k;

				SetModulePoint( x , y , iModuleType );
			}
		}

	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�ʒu���킹�p�^�[���̃Z�b�g
///////////////////////////////////////////////////////////
bool CQRMatrix::SetPositionCheckPattern( int & iErrorType )
{
	bool bRet = true;

	char cPatterns[5][6]
					={"11111"
					 ,"10001"
					 ,"10101"
					 ,"10001"
					 ,"11111"};

	//�^�Ԃɂ��ʒu���킹�p�^�[���̏c���������[�v
	int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;
	for( int i=0 ; c_sPositionChecks[iModelNumber-1][i] ; i++ )
	{
		for( int j=0 ; j<c_sPositionChecks[iModelNumber-1][j] ; j++ )
		{
			//�ʒu���o�p�^�[���Ƃ̔�肪�Ȃ����ǂ������`�F�b�N����
			bool bOutput = true;

			//�����
			int x = c_sPositionChecks[iModelNumber-1][i] - 2;
			int y = c_sPositionChecks[iModelNumber-1][j] - 2;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			//�E���
			x += 4;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			//�E����
			y += 4;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			//������
			x -= 4;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			if( bOutput )
			{
				//�e�ʒu���킹�p�^�[���̏o�͗p�{�b�N�X���[�v
				for( int k = 0 ; k < 5 ; k++ )
				{
					for( int l = 0 ; l < 5 ; l++ )
					{
						int x = c_sPositionChecks[iModelNumber-1][i] + k - 2;
						int y = c_sPositionChecks[iModelNumber-1][j] + l - 2;

						//���������̔���
						int iModuleType = QR_POSITIONCHECK_LIGHT;
						if( cPatterns[k][l] == '1' )
						{
							iModuleType = QR_POSITIONCHECK_DARK;
						}
						SetModulePoint( x , y , iModuleType );

					}
				}
			}
		}
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�^�C�~���O�p�^�[���̃Z�b�g
///////////////////////////////////////////////////////////
bool CQRMatrix::SetTimingPattern( int & iErrorType )
{
	bool bRet = true;

	for( int i=0 ; i<m_iModuleNumber ; i++ )
	{
		int iModuleType = ((i%2)?(QR_TIMING_LIGHT):(QR_TIMING_DARK));

		int x = i;
		int y = 6;
		if( GetModulePoint( x , y ) == QR_UNKNOWN )
		{
			SetModulePoint( x , y , iModuleType );
		}

		x = 6;
		y = i;
		if( GetModulePoint( x , y ) == QR_UNKNOWN )
		{
			SetModulePoint( x , y , iModuleType );
		}

	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�`�����������o��
///		�����F
///			bReserveOnly	�F	true���w�肷��ƃf�[�^�̈�
///								�̗\����s���i���ׂ�LIGHT�œh��Ԃ��j
///////////////////////////////////////////////////////////
bool CQRMatrix::SetTypeInformation( bool bReserveOnly , int & iErrorType )
{
	//�\��݂̂̏ꍇ
	if( bReserveOnly )
	{
		//����̉�����
		for( int i=0 ; i<9 ; i++ )
		{
			int x = i;
			int y = 8;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		//����̏c����
		for( int i = 0 ; i < 8 ; i++ )
		{
			int x = 8;
			int y = i;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		//�E��̉�����
		for( int i=0 ; i<8 ; i++ )
		{
			int x = m_iModuleNumber - 8 + i;
			int y = 8;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		//�����̏c����
		for( int i=0 ; i<8 ; i++ )
		{
			int x = 8;
			int y = m_iModuleNumber - 8 + i;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		return true;
	}

	//�{�ԏo��
	unsigned char ucDataBits[15 + 1] = "000000000000000";

	//�G���[���������̎擾

	unsigned char ucTypeData = (unsigned char)m_iQRMaskPattern & 0x07;
	ucTypeData |= (((c_typeMatrix + m_iModelTypeIndex)->m_iCollectLevel) << 3) & 0x18;

	//bTypeBits�̓��T���ɃZ�b�g����
	for( int i=0 ; i<5 ; i++ )
	{
		unsigned char ucMask = GetBitMask( 4 - i );
		ucDataBits[i] = ( (ucMask & ucTypeData) != 0 )?('1'):('0');
	}

	unsigned char ucCoefficients[] = "10100110111";

	bool bRet = false;

	//�G���[�r�b�g��ǉ�����
	bRet = GetExpressionMod( ucDataBits , 5 , ucCoefficients , ucDataBits + 5 , 10 );

	//�}�X�N�p�^�[���̓K�p
	unsigned char ucMask[] = "101010000010010";

	for( int i=0 ; i<15 ; i++ )
	{
		if( ucDataBits[i] == ucMask[i] )
		{
			ucDataBits[i] = '0';
		}
		else
		{
			ucDataBits[i] = '1';
		}
	}

	//�o�͏���
	
	//���㉡
	int iTemp = 0;
	for( int i = 0 ; i < 8 ; i++,iTemp++ )
	{
		//�^�C�~���O�p�^�[���ɏd�Ȃ�ꍇ
		if( i == 6 )
		{
			iTemp++;
		}

		int iPointData = QR_TYPE_DARK;
		if( ucDataBits[i] == '0' )
		{
			iPointData = QR_TYPE_LIGHT;
		}
		SetModulePoint( iTemp , 8 , iPointData );
	}

	//����c
	iTemp = 0;
	for( int i = 0 ; i < 7 ; i++,iTemp++ )
	{
		//�^�C�~���O�p�^�[���ɏd�Ȃ�ꍇ
		if( i == 6 )
		{
			iTemp++;
		}

		int iPointData = QR_TYPE_DARK;
		if( ucDataBits[14-i] == '0' )
		{
			iPointData = QR_TYPE_LIGHT;
		}
		SetModulePoint( 8 , iTemp , iPointData );
	}

	//����
	for( int i = 0 ; i < 8 ; i++ )
	{
		int iPointData = QR_TYPE_DARK;
		if( i != 7 && ucDataBits[i] == '0' )
		{
			iPointData = QR_TYPE_LIGHT;
		}

		SetModulePoint( 8 , m_iModuleNumber - i - 1 , iPointData );
	}

	//�E��
	for( int i = 0 ; i < 8 ; i++ )
	{
		int iPointData = QR_TYPE_DARK;
		if( ucDataBits[14-i] == '0' )
		{
			iPointData = QR_TYPE_LIGHT;
		}

		SetModulePoint( m_iModuleNumber - i - 1 , 8 , iPointData );
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		�r�b�g�������̏�]�����߂�
///		puszCoefficients�̒�����iErrorLength+1�ƂȂ�
///////////////////////////////////////////////////////////
bool CQRMatrix::GetExpressionMod( unsigned char *puszBaseData , int iBaseDataLength,
						   unsigned char *puszCoefficients ,
						   unsigned char *puszError , int iErrorLength )
{
	bool bRet = true;
	int iCoefficientsLength = iErrorLength + 1;

	size_t sizeDataLength = iBaseDataLength + iCoefficientsLength + 1;
	unsigned char * puszData = new unsigned char[sizeDataLength];

	//�f�[�^������
	memset( puszData , '0' , sizeDataLength - 1 );
	puszData[sizeDataLength - 1] = '\0';

#ifdef WIN32
	memcpy_s( puszData , sizeDataLength , puszBaseData , iBaseDataLength );
#else
	memcpy( puszData , puszBaseData , iBaseDataLength );
#endif

	for( int iKeta = 0 ; iKeta < iBaseDataLength ; iKeta++ )
	{
		if( puszData[iKeta] == '0' )
		{
			continue;
		}

		for( int i=0 ; i<iCoefficientsLength ; i++ )
		{
			if( puszData[iKeta + i] == puszCoefficients[i] )
			{
				puszData[iKeta + i] = '0';
			}
			else
			{
				puszData[iKeta + i] = '1';
			}
		}
	}

#ifdef WIN32
	memcpy_s( puszError , iErrorLength , puszData + iBaseDataLength , iErrorLength );
#else
	memcpy( puszError , puszData + iBaseDataLength , iErrorLength );
#endif

	delete[] puszData;

	return bRet;
}

///////////////////////////////////////////////////////////
///		�^�ԏ�����������
///////////////////////////////////////////////////////////
bool CQRMatrix::SetModelInformation( int & iErrorType )
{
	//�^�ԏ���7�^�����̏ꍇ�i�^�ԏ����L�q���Ȃ��j
	if( (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber < 7 )
	{
		return true;
	}

	unsigned char ucDataBits[18 + 1] = "000000000000000000";
	unsigned char ucModel = (unsigned char)((c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber) & 0x3f;

	for( int i=0 ; i<6 ; i++ )
	{
		unsigned char ucMask = GetBitMask( 5 - i );
		if( ucModel & ucMask )
		{
			ucDataBits[i] = '1';
		}
	}

	unsigned char ucCoefficients[] = "1111100100101";
	bool bRet = GetExpressionMod( ucDataBits , 6 , ucCoefficients , ucDataBits + 6 , 12 );

	//�f�[�^�̏o��
	for( int i=0 ; i<6 ; i++ )
	{
		for( int j=0 ; j<3 ; j++ )
		{
			int iPointData = QR_MODEL_DARK;
			if( ucDataBits[(5 - i) * 3 + (2 - j)] == '0' )
			{
				iPointData = QR_MODEL_LIGHT;
			}

			//�����̌^�ԃf�[�^���o��
			int x = i;
			int y = j + m_iModuleNumber - 11;
			SetModulePoint( x , y , iPointData );

			//�E���̌^�ԃf�[�^���o��
			x = j + m_iModuleNumber - 11;
			y = i;
			SetModulePoint( x , y , iPointData );
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
///		�}�X�N�p�^�[���̓K���̈�̎擾
///		���݂̃��W���[���̈���ŁAQR_UNKNOWN�łȂ�������
///		�}�X�N�p�^�[���K���̈�Ƃ��ăZ�b�g����
///////////////////////////////////////////////////////////
bool CQRMatrix::CreateMaskPatternArea( int & iErrorType )
{
	for( int i=0 ; i<m_iModuleNumber ; i++ )
	{
		for( int j=0 ; j<m_iModuleNumber ; j++ )
		{
			if( m_ppucQRMatrix[i][j] == QR_UNKNOWN )
			{
				m_ppucMaskMatrix[i][j] = QR_MASK_LIGHT;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
///		�w�肳�ꂽ�C���f�b�N�X�̃r�b�g���擾����
///		�߂�l�FON�F0x01
///				OFF�F0x00
///////////////////////////////////////////////////////////
unsigned char CQRMatrix::GetDataBit( int iBitIndex , bool bDataRequest )
{
	int iByteIndex = iBitIndex / 8;

	//�f�[�^�̃C���f�b�N�X�����l�̏ꍇ
	if( iByteIndex < 0 )
	{
		return (unsigned char)0x00;
	}

	//�}�X�N�̎擾
	unsigned char ucMask = GetBitMask( 7 - iBitIndex % 8 );

	//�f�[�^�����N�G�X�g����Ă���ꍇ
	if( bDataRequest )
	{
		if( iByteIndex < m_iDataSizeNumber && m_pucData[iByteIndex] & ucMask )
		{
			return (unsigned char)0x01;
		}
	}
	//�����������N�G�X�g����Ă���ꍇ
	else
	{
		if( iByteIndex < m_iErrorSizeNumber && m_pucError[iByteIndex] & ucMask )
		{
			return (unsigned char)0x01;
		}
	}

	return (unsigned char)0x00;
}

///////////////////////////////////////////////////////////
///		�f�[�^�r�b�g�p�^�[�����o�͂���
///////////////////////////////////////////////////////////
bool CQRMatrix::OutputDataBits( int & iErrorType )
{
	int iColumnIndex = m_iModuleNumber - 1;
	int iRowIndex    = m_iModuleNumber - 1;

	bool bDirectionUp = true;
	bool bFirstColumn = true;

	int iRSBlockNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iRSBlockNumber;
	int iDataByteNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber / 8;
	int iBitIndex = 0;
	int iFirstCount , iSecondCount;
	int iFirstDataNumber , iSecondDataNumber;
	
	SeparateTotalNumber( iRSBlockNumber , iDataByteNumber 
						, iFirstCount , iFirstDataNumber 
						, iSecondCount , iSecondDataNumber );

	bool bRet = true;

	//�u���b�N���Ƃ̃f�[�^���[�v
	for( int iBlockIndex=0 ; iBlockIndex<iFirstDataNumber+1 ; iBlockIndex++ )
	{
		//�����u���b�N�̏o��
		for( int iSeq = 0 ; iSeq < iRSBlockNumber ; iSeq++ )
		{
			int iByteIndex = 0;

			//�t�@�[�X�g�J�E���g���̏ꍇ
			if( iSeq < iFirstCount )
			{
				iByteIndex = iFirstDataNumber * iSeq;

				//�u���b�N���C���f�b�N�X���f�[�^�̈�����傫���ꍇ
				if( iBlockIndex >= iFirstDataNumber )
				{
					continue;
				}
			}
			else
			{
				iByteIndex = iFirstDataNumber * iFirstCount + (iSeq - iFirstCount) * iSecondDataNumber;
			}

			iByteIndex += iBlockIndex;
			iBitIndex = iByteIndex * 8;

			for( int i=0 ; i<8 ; i++ )
			{
				GetNextDataPutPoint( iColumnIndex , iRowIndex , bFirstColumn , bDirectionUp );

				if( GetDataBit( iBitIndex , true ) )
				{
					bRet = SetModulePoint( iColumnIndex , iRowIndex , QR_DATA_DARK );
				}
				else
				{
					bRet = SetModulePoint( iColumnIndex , iRowIndex , QR_DATA_LIGHT );
				}

				iBitIndex++;
			}
		}
	}

	int iErrorByteNumber = ( (c_typeMatrix + m_iModelTypeIndex)->m_iTotalBits - 
							 (c_typeMatrix + m_iModelTypeIndex)->m_iDataBitNumber) / 8 / iRSBlockNumber;
	//�u���b�N���Ƃ̃f�[�^�̃��[�v
	for( int iBlockIndex=0 ; iBlockIndex < iErrorByteNumber ; iBlockIndex++ )
	{
		//�q�r�u���b�N���Ƃ̃f�[�^�̃��[�v
		for( int iSeq = 0 ; iSeq < iRSBlockNumber ; iSeq++ )
		{
			int iByteIndex = iSeq * iErrorByteNumber + iBlockIndex;
			iBitIndex = iByteIndex * 8;

			for( int i=0 ; i<8 ; i++ )
			{
				GetNextDataPutPoint( iColumnIndex , iRowIndex , bFirstColumn , bDirectionUp );

				if( GetDataBit( iBitIndex , false ) )
				{
					bRet = SetModulePoint( iColumnIndex , iRowIndex , QR_COLLECT_DARK );
				}
				else
				{
					bRet = SetModulePoint( iColumnIndex , iRowIndex , QR_COLLECT_LIGHT );
				}

				iBitIndex++;
			}
		}
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////
///		���̃f�[�^�̃Z�b�g�|�C���g���擾����
/////////////////////////////////////////////////////////////////////
bool CQRMatrix::GetNextDataPutPoint( int & iColumnIndex , int & iRowIndex
									, bool & bFirstColumn
									, bool & bDirectionUp )
{
	//iColumnIndex�����iRowIndex�̈ʒu����
	while( GetModulePoint( iColumnIndex , iRowIndex ) != QR_UNKNOWN )
	{
		bool bDirectionChanged = false;
		//�Q��̍ŏ��̃f�[�^�̏ꍇ
		if( bFirstColumn )
		{
			iColumnIndex--;
			bFirstColumn = false;
		}
		//�c������ύX����ꍇ
		else
		{
			//��Ɉړ�����ꍇ
			if( bDirectionUp )
			{
				//��̍s��
				iRowIndex--;

				//�ŏ�i�𒴂����ꍇ
				if( iRowIndex < 0 )
				{
					iRowIndex = 0;
					iColumnIndex--;
					bDirectionUp = false;
				}
				else
				{
					iColumnIndex++;
				}
			}
			//���Ɉړ�����ꍇ
			else
			{
				//���̍s��
				iRowIndex++;

				//�ŉ��i�𒴂����ꍇ
				if( iRowIndex >= m_iModuleNumber )
				{
					iRowIndex = m_iModuleNumber - 1;
					iColumnIndex--;
					bDirectionUp = true;
				}
				else
				{
					iColumnIndex++;
				}
			}

			//����i�񂪁A�^�C�~���O�p�^�[���i�c�̗�̏ꍇ�j
			if( iColumnIndex == 6 )
			{
				iColumnIndex = 5;
			}

			bFirstColumn = true;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////
///		�}�X�N�p�^�[���̌���
///////////////////////////////////////////////////////////
bool CQRMatrix::DecideMaskPattern( int & iErrorType )
{
	int iMinLostPoint;
	int iMinQRMaskPattern;
	bool bRet = true;

	for( m_iQRMaskPattern = 0 ; m_iQRMaskPattern < 8 ; m_iQRMaskPattern++ )
	{
		int iLostPoint = 0;

		int iBlackPointNumber = 0;
		
		//�񂲂Ƃ̃`�F�b�N
		for( int iRowIndex = 0 ; iRowIndex < m_iModuleNumber ; iRowIndex++ )
		{
			int iSeqBlackModuleNumber = 0;
			int iSeqWhiteModuleNumber = 0;

			for( int iColumnIndex = 0 ; iColumnIndex < m_iModuleNumber ; iColumnIndex++ )
			{
				//�Ã|�C���g�̏ꍇ
				if( IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
					iSeqBlackModuleNumber++;
					iBlackPointNumber++;

					//�A�����|�C���g�̃`�F�b�N
					iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
					iSeqWhiteModuleNumber = 0;
				}
				//���|�C���g�̏ꍇ
				else
				{
					iSeqWhiteModuleNumber++;

					//�A���Ã|�C���g�̃`�F�b�N
					iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
					iSeqBlackModuleNumber = 0;
				}

				//�Q�~�Q�u���b�N�̎��_�J�E���g
				iLostPoint += GetLostPoint_ModuleBlock( iColumnIndex , iRowIndex );

				//1�F1�F3�F1�F1�䗦�̃`�F�b�N
				iLostPoint += GetLostPoint_PositionDitect( iColumnIndex , iRowIndex );
			}

			//�A���Ã|�C���g�̃`�F�b�N
			iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
			//�A�����|�C���g�̃`�F�b�N
			iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
		}

		//�Á^���䗦
		iLostPoint += GetLostPoint_BlackWhiteRatio( iBlackPointNumber );

		//�s���Ƃ̃`�F�b�N
		for( int iColumnIndex = 0 ; iColumnIndex < m_iModuleNumber ; iColumnIndex++ )
		{
			int iSeqBlackModuleNumber = 0;
			int iSeqWhiteModuleNumber = 0;

			//�񂲂Ƃ̃`�F�b�N
			for( int iRowIndex = 0 ; iRowIndex < m_iModuleNumber ; iRowIndex++ )
			{
				//�Ã|�C���g�̏ꍇ
				if( IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
					iSeqBlackModuleNumber++;

					//�A�����|�C���g�̃`�F�b�N
					iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
					iSeqWhiteModuleNumber = 0;
				}
				//���|�C���g�̏ꍇ
				else
				{
					iSeqWhiteModuleNumber++;
					
					//�A���Ã|�C���g�̃`�F�b�N
					iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
					iSeqBlackModuleNumber = 0;
				}
			}

			//�A���Ã|�C���g�̃`�F�b�N
			iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
			//�A�����|�C���g�̃`�F�b�N
			iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
		}

		//��Ԏ��_�̏��Ȃ��}�X�N�p�^�[���̎擾
		if( m_iQRMaskPattern == 0 )
		{
			iMinLostPoint = iLostPoint;
			iMinQRMaskPattern = 0;
		}
		else if( iMinLostPoint > iLostPoint )
		{
			iMinLostPoint = iLostPoint;
			iMinQRMaskPattern = m_iQRMaskPattern;
		}
	}

	m_iQRMaskPattern = iMinQRMaskPattern;

	return bRet;
}

///////////////////////////////////////////////////////////
///		���_�̃J�E���g�i�A���ÁE���j
///////////////////////////////////////////////////////////
int CQRMatrix::GetLostPoint_SeqModule( int iSeqBlock )
{
	if( iSeqBlock < 5 )
	{
		return 0;
	}

	return (iSeqBlock - 5) + 3;
}

///////////////////////////////////////////////////////////
///		���_�̃J�E���g�i�w���E�s���J�n�_�Ƃ����Q�~�Q�u���b�N�j
///////////////////////////////////////////////////////////
int CQRMatrix::GetLostPoint_ModuleBlock( int iColumnIndex , int iRowIndex )
{
	if( iColumnIndex < m_iModuleNumber - 1 && iRowIndex < m_iModuleNumber - 1 )
	{
		return 0;
	}

	bool bDark = IsDarkPoint( iColumnIndex , iRowIndex );
	if( bDark == IsDarkPoint( iColumnIndex + 1 , iRowIndex     ) &&
		bDark == IsDarkPoint( iColumnIndex     , iRowIndex + 1 ) &&
		bDark == IsDarkPoint( iColumnIndex + 1 , iRowIndex + 1 ) )
	{
		return 3;
	}
	return 0;
}

/////////////////////////////////////////////////////////////
///		���_�̃J�E���g�i�w��ʒu����1�F1�F3�F1�F1�̍s��`�F�b�N
///						�}�b�`����ꍇ�ɂ͑O��̖��`�F�b�N�j
/////////////////////////////////////////////////////////////
int CQRMatrix::GetLostPoint_PositionDitect( int iColumnIndex , int iRowIndex )
{
	bool bDirectionColumn = true;
	int iLostPoint = 0;

	//�c�����[�v
	for( int i=0 ; i<2 ; i++,bDirectionColumn = false )
	{
		//������������ɂV�h�b�g�ȏ�̃f�[�^�����݂��Ȃ��ꍇ
		if( (bDirectionColumn && iColumnIndex >= m_iModuleNumber - 7) ||
			(!bDirectionColumn && iRowIndex >= m_iModuleNumber - 7 ) )
		{
			continue;
		}

		//1�F1�F3�F1�F1�̃`�F�b�N
		bool bMatched = true;

		for( int iIndex = 0 ; iIndex<7 ; iIndex++ )
		{
			bool bNeedDark = false;
			if( iIndex == 3 || iIndex % 2 == 0 )
			{
				bNeedDark = true;
			}

			if( IsDarkPoint( iColumnIndex + ((bDirectionColumn)?(iIndex):(0))
							,iRowIndex    + ((bDirectionColumn)?(0):(iIndex)) ) != bNeedDark )
			{
				bMatched = false;
				break;
			}
		}

		//1�F1�F3�F1�F1�����݂��Ȃ��ꍇ
		if( !bMatched )
		{
			continue;
		}

		//�O�̃X�y�[�X���J�E���g����
		if( ( bDirectionColumn && iColumnIndex > 3) || 
			(!bDirectionColumn && iRowIndex > 3 ) )
		{
			bMatched = true;

			for( int iIndex = 0 ; iIndex < 4 ; iIndex++ )
			{
				int iPlusCount = -iIndex - 1;
				if( IsDarkPoint( iColumnIndex + ((bDirectionColumn)?(iPlusCount):(0))
								,iRowIndex    + ((bDirectionColumn)?(0):(iPlusCount)) ) )
				{
					bMatched = false;
					break;
				}
			}

			//�O�S�����̏ꍇ
			if( bMatched )
			{
				iLostPoint += 40;
				continue;
			}
		}

		//���̃X�y�[�X���J�E���g����
		if( ( bDirectionColumn && iColumnIndex < m_iModuleNumber - 7 - 4 ) ||
			(!bDirectionColumn && iRowIndex    < m_iModuleNumber - 7 - 4 ) )
		{
			bMatched = true;

			for( int iIndex = 0 ; iIndex < 4 ; iIndex++ )
			{
				int iPlusCount = iIndex + 7 + i;
				if( IsDarkPoint( iColumnIndex + ((bDirectionColumn)?(iPlusCount):(0))
								,iRowIndex    + ((bDirectionColumn)?(0):(iPlusCount)) ) )
				{
					bMatched = false;
					break;
				}
			}

			//���S�����̏ꍇ
			if( bMatched )
			{
				iLostPoint += 40;
			}
		}

	}
	return iLostPoint;
}

///////////////////////////////////////////////////////////
///		���_�̃J�E���g�i�ÁE���䗦�j
///////////////////////////////////////////////////////////
int CQRMatrix::GetLostPoint_BlackWhiteRatio( int iBlackPoint )
{
	int iAllNumber = m_iModuleNumber * m_iModuleNumber;
	int iMod = iAllNumber / 2 - iBlackPoint;
	if( iMod < 0 )
	{
		iMod = -iMod;
	}

	return ( ( (iMod * 100 + iAllNumber - 1) / iAllNumber) / 5 ) * 10;
}
