#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>

#include "QRReadSolomon.h"
#include "QRReadSolomonInformation.h"

CQRReadSolomon::CQRReadSolomon()
{
	m_pNumberAlpha = new SAlphaNumber[ 256 ];
	for( int i=0 ; i<256 ; i++ )
	{
		m_pNumberAlpha[i] = c_alphaNumber[i];
	}
	qsort( m_pNumberAlpha , 256 , sizeof(SAlphaNumber) , SortCompareNumberAlpha );

	(m_pNumberAlpha  )->m_sNumber = 0;
	(m_pNumberAlpha  )->m_sAlpha = 0;

	(m_pNumberAlpha+1)->m_sNumber = 1;
	(m_pNumberAlpha+1)->m_sAlpha = 0;

}

CQRReadSolomon::~CQRReadSolomon()
{
	delete[] m_pNumberAlpha;
}

int CQRReadSolomon::SortCompareNumberAlpha( const void * pcParam1 , const void * pcParam2 )
{
	SAlphaNumber * pAlphaNumber1 = (SAlphaNumber*)pcParam1;
	SAlphaNumber * pAlphaNumber2 = (SAlphaNumber*)pcParam2;

	return pAlphaNumber1->m_sNumber - pAlphaNumber2->m_sNumber;
}

///////////////////////////////////////////////////////////
///		���[�h�\�������������̎��s
///		�����F
///			pucBase		�F	(IN)	�R�[�h�����s���f�[�^��
///			iDataLength	�F	(IN)	�R�[�h�����s���f�[�^��
///			pucCode		�F	(OUT)	�R�[�h�����ʂ̊i�[�p�z��
///			iCodeLength	�F	(IN)	�R�[�h�����ʂ̊i�[�p�z��v�f�����R�[�h�ꐔ
///////////////////////////////////////////////////////////
bool CQRReadSolomon::CodeTo( unsigned char * pucBase , int iBaseLength
							,unsigned char * pucCode , int iCodeLength )
{
	bool bRet = true;

	//�����������̌W�����擾����
	unsigned char * pucCoefficients = GetExpressionCoefficients( iCodeLength );

	if( pucCoefficients == NULL )
	{
		return false;
	}

	//�x�[�X�f�[�^�̕������쐬����
	int iDataLength = iBaseLength + iCodeLength;
	unsigned char * pucData = new unsigned char[iDataLength];
	memset( pucData , 0 , iDataLength );
#ifdef WIN32
	memcpy_s( pucData , iDataLength , pucBase , iBaseLength );
#else
	memcpy( pucData , pucBase , iBaseLength );
#endif

	int iDataLast = iDataLength - iCodeLength;
	for( int iDataIndex = 0; iDataIndex < iDataLast ; iDataIndex++ )
	{
		//�f�[�^�W��
		short sKeisu = (short)pucData[iDataIndex];

		//�ΏۂƂȂ錅������0�̏ꍇ
		if( !sKeisu )
		{
			continue;
		}
		
		//�W���ɑ΂���A���t�@�̗ݏ���擾����
		short sAlpha = (m_pNumberAlpha + sKeisu )->m_sAlpha ;

		//�����������̌W�����X�g�Ƀ��ݏ�����Z����i�ݏ擯�m�̊|���Z�����搔�̑����Z�j
		for( int i=0 ; i<iCodeLength+1 ; i++ )
		{
			short sTemp = (short)pucCoefficients[i] + sAlpha;

			//���O255==1���A���𗎂Ƃ�
			while( sTemp > 255 )
			{
				sTemp -= 255;
			}

			//�e���̘_���a���擾���A�f�[�^�r�b�g�𖄂߂�
			pucData[iDataIndex + i] = (pucData[iDataIndex + i]) ^ ((c_alphaNumber + sTemp)->m_sNumber);
		}

	}

	//��]�f�[�^��z��ɃZ�b�g����
	for( int i=0 ; i<iCodeLength ; i++ )
	{
		int iDataIndex = iDataLength - i - 1;
		int iCodeIndex = iCodeLength - i - 1;

		pucCode[iCodeIndex] = pucData[iDataIndex];
	}

	//�����������̌W�����X�g���폜����
	delete[] pucCoefficients;
	delete[] pucData;

	return bRet;
}

///////////////////////////////////////////////////////////
///		�����������̌W�����擾����
///		�����F
///			iCodeLength	�F	(IN)	�R�[�h�ꐔ
///		�߂�l�F
///			�W�����̗ݏ惊�X�g�ipucCoefficients�j
///			x^iCodeLength      *��^(pucCoefficients[0])
///			+ x^(iCodeLength-1)*��^(pucCoefficients[1])
///			+ x^(iCodeLength-2)*��^(pucCoefficients[2])
///			+ �c�c�c�c
///			+ x                *��^(pucCoefficients[iCodeLength-1])
///			+ 1                *��^(pucCoefficients[iCodeLength  ])
///
///			���i^�j�͗ݏ���Ӗ�����Bxor�ł͂Ȃ��̂ŔO�̂���
///////////////////////////////////////////////////////////
unsigned char * CQRReadSolomon::GetExpressionCoefficients( int iCodeLength )
{
	int iExpressionIndex = 0;
	for( ; (c_readSolomonExpression + iExpressionIndex)->m_iCodeNumber != iCodeLength && 
		   (c_readSolomonExpression + iExpressionIndex)->m_iCodeNumber >= 0
		 ; iExpressionIndex++ );

	//�������������̃��X�g�Ɏw��̃R�[�h�ꐔ�f�[�^�����݂��Ȃ��ꍇ
	if( (c_readSolomonExpression + iExpressionIndex)->m_iCodeNumber != iCodeLength )
	{
		return NULL;
	}

	unsigned char * pucCoefficients = new unsigned char[iCodeLength + 1];
	
	//�������X�g�̃��[�v
	
	size_t sizeLength = strlen( (c_readSolomonExpression + iExpressionIndex)->m_pcszExpression) + 1;
	char * pszExpression = new char[sizeLength];
#ifdef WIN32
	strcpy_s( pszExpression , sizeLength , (c_readSolomonExpression + iExpressionIndex)->m_pcszExpression );
#else
	strcpy( pszExpression , (c_readSolomonExpression + iExpressionIndex)->m_pcszExpression );
#endif

	//�J���}��؂���P�������X�g�ɕύX����
	for( int i=0 ; i<(int)sizeLength ; i++ )
	{
		if( *(pszExpression + i) == ',' )
		{
			*(pszExpression + i) = '\0';
		}
	}

	int iIndex = 0;
	for( int iStringIndex = 0
		; pszExpression[iStringIndex] && iIndex < iCodeLength + 1
		; iStringIndex += (int)strlen( pszExpression + iStringIndex ) + 1, iIndex++ )
	{
		pucCoefficients[iIndex] = (unsigned char)atoi( pszExpression + iStringIndex);
	}

	delete[] pszExpression;

	return pucCoefficients;
}
