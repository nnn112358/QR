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
///		リードソロモン符号化の実行
///		引数：
///			pucBase		：	(IN)	コード化を行うデータ列
///			iDataLength	：	(IN)	コード化を行うデータ数
///			pucCode		：	(OUT)	コード化結果の格納用配列
///			iCodeLength	：	(IN)	コード化結果の格納用配列要素数＝コード語数
///////////////////////////////////////////////////////////
bool CQRReadSolomon::CodeTo( unsigned char * pucBase , int iBaseLength
							,unsigned char * pucCode , int iCodeLength )
{
	bool bRet = true;

	//生成多項式の係数を取得する
	unsigned char * pucCoefficients = GetExpressionCoefficients( iCodeLength );

	if( pucCoefficients == NULL )
	{
		return false;
	}

	//ベースデータの複製を作成する
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
		//データ係数
		short sKeisu = (short)pucData[iDataIndex];

		//対象となる桁が既に0の場合
		if( !sKeisu )
		{
			continue;
		}
		
		//係数に対するアルファの累乗を取得する
		short sAlpha = (m_pNumberAlpha + sKeisu )->m_sAlpha ;

		//生成多項式の係数リストにα累乗を加算する（累乗同士の掛け算＝＞乗数の足し算）
		for( int i=0 ; i<iCodeLength+1 ; i++ )
		{
			short sTemp = (short)pucCoefficients[i] + sAlpha;

			//α＾255==1より、桁を落とす
			while( sTemp > 255 )
			{
				sTemp -= 255;
			}

			//各桁の論理和を取得し、データビットを埋める
			pucData[iDataIndex + i] = (pucData[iDataIndex + i]) ^ ((c_alphaNumber + sTemp)->m_sNumber);
		}

	}

	//剰余データを配列にセットする
	for( int i=0 ; i<iCodeLength ; i++ )
	{
		int iDataIndex = iDataLength - i - 1;
		int iCodeIndex = iCodeLength - i - 1;

		pucCode[iCodeIndex] = pucData[iDataIndex];
	}

	//生成多項式の係数リストを削除する
	delete[] pucCoefficients;
	delete[] pucData;

	return bRet;
}

///////////////////////////////////////////////////////////
///		生成多項式の係数を取得する
///		引数：
///			iCodeLength	：	(IN)	コード語数
///		戻り値：
///			係数αの累乗リスト（pucCoefficients）
///			x^iCodeLength      *α^(pucCoefficients[0])
///			+ x^(iCodeLength-1)*α^(pucCoefficients[1])
///			+ x^(iCodeLength-2)*α^(pucCoefficients[2])
///			+ …………
///			+ x                *α^(pucCoefficients[iCodeLength-1])
///			+ 1                *α^(pucCoefficients[iCodeLength  ])
///
///			※（^）は累乗を意味する。xorではないので念のため
///////////////////////////////////////////////////////////
unsigned char * CQRReadSolomon::GetExpressionCoefficients( int iCodeLength )
{
	int iExpressionIndex = 0;
	for( ; (c_readSolomonExpression + iExpressionIndex)->m_iCodeNumber != iCodeLength && 
		   (c_readSolomonExpression + iExpressionIndex)->m_iCodeNumber >= 0
		 ; iExpressionIndex++ );

	//生成多項式内のリストに指定のコード語数データが存在しない場合
	if( (c_readSolomonExpression + iExpressionIndex)->m_iCodeNumber != iCodeLength )
	{
		return NULL;
	}

	unsigned char * pucCoefficients = new unsigned char[iCodeLength + 1];
	
	//引数リストのループ
	
	size_t sizeLength = strlen( (c_readSolomonExpression + iExpressionIndex)->m_pcszExpression) + 1;
	char * pszExpression = new char[sizeLength];
#ifdef WIN32
	strcpy_s( pszExpression , sizeLength , (c_readSolomonExpression + iExpressionIndex)->m_pcszExpression );
#else
	strcpy( pszExpression , (c_readSolomonExpression + iExpressionIndex)->m_pcszExpression );
#endif

	//カンマ区切りを１次元リストに変更する
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
