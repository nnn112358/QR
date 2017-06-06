#include "QRDataElement.h"
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
///		CQRDataElement�̒�`
///////////////////////////////////////////////////////////////////////////////
CQRDataElement::CQRDataElement()
{
	InitData();
}

CQRDataElement::~CQRDataElement()
{
}

CQRDataElement::CQRDataElement( const CQRDataElement & qrElement )
{
	InitData();
	SetData( qrElement.m_pszData , qrElement.m_iDataLength , qrElement.m_iDataMode );
}

void CQRDataElement::InitData()
{
	m_iDataMode = -1;
	m_iDataLength = 0;
	m_pszData = NULL;
}

///////////////////////////////////////////////////////////
///		�G�������g�����f�[�^���Z�b�g����
///////////////////////////////////////////////////////////
void CQRDataElement::SetData( const char * pcszString , int iDataLength, int iDataMode )
{
	ReleaseData();

	if( pcszString != NULL )
	{
		m_pszData = new char[iDataLength];
#ifdef WIN32
		memcpy_s( m_pszData , iDataLength , pcszString , iDataLength );
#else
		memcpy( m_pszData , pcszString , iDataLength );
#endif
		m_iDataLength = iDataLength;
		m_iDataMode = iDataMode;
	}
}

void CQRDataElement::ReleaseData()
{
	if( m_pszData )
	{
		delete[] m_pszData;
	}
	m_pszData = NULL;
	m_iDataLength = 0;
}

bool CQRDataElement::AddData( const char * pcszString , int iDataLength , int iDataMode )
{
	//�܂��f�[�^���o�^����Ă��Ȃ��ꍇ
	if( m_pszData == NULL )
	{
		SetData( pcszString , iDataLength , iDataMode );
		return true;
	}

	//�f�[�^���[�h���قȂ�ꍇ�i�߂�j
	if( !pcszString || iDataMode != m_iDataMode )
	{
		return false;
	}

	//�f�[�^�̈�̊m��
	int iMemorySize = iDataLength + m_iDataLength;
	char * pszTemp = new char[iMemorySize];

	//�f�[�^�̒ǉ��R�s�[
#ifdef WIN32
	memcpy_s( pszTemp , iMemorySize , m_pszData , m_iDataLength );
	memcpy_s( pszTemp+m_iDataLength , iDataLength , pcszString , iDataLength );
#else
	memcpy( pszTemp , m_pszData , m_iDataLength );
	memcpy( pszTemp + m_iDataLength , pcszString , iDataLength );
#endif

	ReleaseData();
	m_iDataLength = iMemorySize;
	m_pszData = pszTemp;

	return true;
}