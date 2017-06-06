#include "QRDataElement.h"
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
///		CQRDataElementの定義
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
///		エレメント内部データをセットする
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
	//まだデータが登録されていない場合
	if( m_pszData == NULL )
	{
		SetData( pcszString , iDataLength , iDataMode );
		return true;
	}

	//データモードが異なる場合（戻る）
	if( !pcszString || iDataMode != m_iDataMode )
	{
		return false;
	}

	//データ領域の確保
	int iMemorySize = iDataLength + m_iDataLength;
	char * pszTemp = new char[iMemorySize];

	//データの追加コピー
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