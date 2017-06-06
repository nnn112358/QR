
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "QRMatrix.h"

#include "QRInformation.h"
#include "QRReadSolomon.h"

///////////////////////////////////////////////////////////////////////////////
///		CQRMatrixの定義
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
///		内部リストに文字列データをセットする
///////////////////////////////////////////////////////////
bool CQRMatrix::AddQRCharacters( const char * pcszString , int iCharacterType )
{
	CQRDataElement qrElement;

	bool bDataAdded = false;

	//１つ以上のデータが登録されている場合
	if( m_listQRData.begin() != m_listQRData.end() )
	{
		list<CQRDataElement>::iterator pQRElement = m_listQRData.end();
		pQRElement--;

		//最後のデータモードと現在のデータモードが一致している場合には最後のリストに
		//文字列を追加する
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
///		全データを削除する
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
///		QRのデータを削除する
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
///		マスクのデータを削除する
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
///		セットされているデータを元にＱＲマトリックスを作成する
///		引数：
///			iCollectLevel	：	誤り訂正レベル
///									COLLECTLEVEL_L
///									COLLECTLEVEL_M
///									COLLECTLEVEL_Q
///									COLLECTLEVEL_H
///								のいずれかを指定する
///		戻り値：
///			==false：作成に失敗（データが大きすぎるなど）
///			==true：作成に成功
///////////////////////////////////////////////////////////
bool CQRMatrix::MakeQRMatrix( int iCollectLevel , int & iErrorType )
{
	//現在設定されているQRデータ領域を削除する
	ReleaseAll();

	bool bRet = true;
	iErrorType = QRERR_NOERROR;

	//型番情報を設定する
	bRet = SetTypeNumber( iCollectLevel , iErrorType );

	//データビット列を作成する
	if( bRet )
	{
		bRet = CreateDataBits( iErrorType );
	}

	//誤り訂正コード文字列の作成
	if( bRet )
	{
		bRet = CreateErrorCollectData( iErrorType );
	}

	//セットされた型番号から、モジュール配列を初期化する
	if( bRet )
	{
		bRet = InitModuleArea( iErrorType );
	}

	//位置検出パターンのセット
	if( bRet )
	{
		bRet = SetPositionDitectPattern( iErrorType );
	}

	//位置あわせパターンのセット
	if( bRet )
	{
		bRet = SetPositionCheckPattern( iErrorType );
	}

	//タイミングパターンのセット
	if( bRet )
	{
		bRet = SetTimingPattern( iErrorType );
	}

	//形式情報の仮出力
	if( bRet )
	{
		bRet = SetTypeInformation( true , iErrorType );
	}

	//型番情報
	if( bRet )
	{
		bRet = SetModelInformation( iErrorType );
	}

	//マスクパターン適応領域を取得する
	if( bRet )
	{
		bRet = CreateMaskPatternArea( iErrorType );
	}

	//データビットの出力
	if( bRet )
	{
		bRet = OutputDataBits( iErrorType );
	}

	//マスクパターンの決定
	if( bRet )
	{
		bRet = DecideMaskPattern( iErrorType );
	}

	//形式情報の出力
	if( bRet )
	{
		bRet = SetTypeInformation( false, iErrorType );
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		登録されているデータからデータ文字列を作成する
///////////////////////////////////////////////////////////
bool CQRMatrix::CreateDataBits( int & iErrorType )
{
	bool bRet = true;

	//データのリリース
	ReleaseData();

	list<CQRDataElement>::iterator pDataElement;
	for( pDataElement = m_listQRData.begin()
		;pDataElement != m_listQRData.end() && bRet 
		;pDataElement ++ )
	{
		//データモードの出力
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


	//終端出力
	if( bRet )
	{
		bRet = AddDataBits( (unsigned char)0x00 , 4 , iErrorType);
	}

	if( bRet )
	{
		//データバイト終端までを0で埋める
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
///		登録済みのデータリストからデータのビット数を取得する
///		ＱＲの型番により、サイズが異なるため、各型番時のデータサイズを
///		セットする
///		引数：
///			iDataBit1_9		(OUT)	1型～9型のデータビット数
///			iDataBit10_26	(OUT)	10型～26型のデータビット数
///			iDataBit27_40	(OUT)	27型～40型のデータビット数
///////////////////////////////////////////////////////////
bool CQRMatrix::GetDataBitNumber( int & iDataBit1_9 , int & iDataBit10_26 , int & iDataBit27_40 )
{
	bool bRet = true;

	//初期化
	iDataBit1_9 = iDataBit10_26 = iDataBit27_40 = 0;
	//型に関係ないデータビット数
	int iDataBits = 0;

	list<CQRDataElement>::iterator pDataElement;
	for( pDataElement = m_listQRData.begin()
		; pDataElement != m_listQRData.end()
		; pDataElement++ )
	{
		//データ指示子
		iDataBits += 4;

		switch( pDataElement->m_iDataMode )
		{
		//数字モードの場合
		case QRCHARACTERTYPE_NUMERIC:

			//データサイズ出力用
			iDataBit1_9   += 10;
			iDataBit10_26 += 12;
			iDataBit27_40 += 14;

			//データ本体出力用
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

		//アルファベットモードの場合
		case QRCHARACTERTYPE_ALPHABET:

			//データサイズ出力用
			iDataBit1_9   += 9;
			iDataBit10_26 += 11;
			iDataBit27_40 += 13;

			//データ本体出力用
			iDataBits += ( pDataElement->m_iDataLength / 2 ) * 11;
			if( pDataElement->m_iDataLength % 2 ) 
			{
				iDataBits += 6;
			}

			break;

		//ASCII文字列
		case QRCHARACTERTYPE_ASCII:

			//データサイズ出力用
			iDataBit1_9 += 8;
			iDataBit10_26 += 16;
			iDataBit27_40 += 16;

			iDataBits += pDataElement->m_iDataLength * 8;
			break;

		//漢字文字列用
		case QRCHARACTERTYPE_KANJI:

			//データサイズ出力用
			iDataBit1_9   += 8;
			iDataBit10_26 += 10;
			iDataBit27_40 += 12;

			//データ出力用
			iDataBits += (pDataElement->m_iDataLength / 2) * 13;

			break;

		default:
			return false;
		}
	}

	//終端モード指示子
	iDataBits += 4;

	iDataBit1_9 += iDataBits;
	iDataBit10_26 += iDataBits;
	iDataBit27_40 += iDataBits;

	return bRet;
}

///////////////////////////////////////////////////////////
///		数字データのセット処理
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Numeric( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//データが数字のみかどうかのチェック
	for( int i=0 ; i<iDataLength ; i++ )
	{
		char cTemp = pcszData[i];

		//数字でない場合は即戻り
		if( !isdigit( cTemp ) )
		{
			iErrorType = QRERR_DATAISNOT_NUMERIC;
			return false;
		}

	}

	//数字の指示子出力
	bRet = AddDataBits( (unsigned char)0x01 , 4 , iErrorType );

	//文字数指示子の出力
	short sOutputNumber = (short)iDataLength;

	int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;

	//１～９型の場合（１０ビット）
	if( iModelNumber <= 9 )
	{
		bRet = AddDataBits( (unsigned char)( (sOutputNumber >> 8) & 0x03 ) , 2 , iErrorType );
	}

	//１０～２６型の場合（１２ビット）
	else if(iModelNumber <= 26 )
	{
		bRet = AddDataBits( (unsigned char)( (sOutputNumber >> 8) & 0x0f ) , 4 , iErrorType );
	}

	//２７～４０型の場合（１４ビット）
	else
	{
		bRet = AddDataBits( (unsigned char)( (sOutputNumber >> 8) & 0x3f ) , 6 , iErrorType );
	}

	//下位８ビット出力
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

		//１文字のみ
		if( sizeLength == 1 )
		{
			bRet = AddDataBits( (unsigned char)(sTemp & 0x0f) , 4 , iErrorType );
		}
		//２文字
		else if( sizeLength == 2 )
		{
			bRet = AddDataBits( (unsigned char)(sTemp & 0x7f) , 7 , iErrorType );
		}
		//３文字
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
///		アルファベットデータのセット処理
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Alphabet( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//データが英数字のみかどうかをチェックする
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

	//アルファベットの指示子出力
	bRet = AddDataBits( (unsigned char)0x02 , 4 , iErrorType );

	//文字数の出力
	unsigned short usLength = (unsigned short)iDataLength;
	int iModelTypeNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;
	
	//１～９型
	if( iModelTypeNumber <= 9 )
	{
		bRet = AddDataBits( (unsigned char)(( usLength >> 8 ) & 0x01) , 1 , iErrorType );
	}
	//１０～２６型
	else if( iModelTypeNumber <= 26 )
	{
		bRet = AddDataBits( (unsigned char)(( usLength >> 8 ) & 0x07) , 3 , iErrorType );
	}
	//２７～４０型
	else
	{
		bRet = AddDataBits( (unsigned char)(( usLength >> 8 ) & 0x1f) , 5 , iErrorType );
	}

	if( bRet )
	{
		bRet = AddDataBits( (unsigned char)(usLength & 0xff) , 8 , iErrorType );
	}

	//２文字ずつ１１ビットに変換する
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

		//１文字のみの出力の場合（６bit出力）
		if( iChar == 1 )
		{
			bRet = AddDataBits( (unsigned char)(usChar & 0x3f) , 6 , iErrorType );
		}
		//２文字の出力の場合（１１bit出力）
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
///		アスキーデータのセット処理
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Ascii( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//アスキーの指示子出力
	bRet = AddDataBits( (unsigned char)0x04 , 4 , iErrorType );
	
	if( bRet )
	{
		//データ長の出力
		int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;

		//１型～９型
		if( iModelNumber <= 9 )
		{
			bRet = AddDataBits( (unsigned char)iDataLength , 8 , iErrorType );
		}
		//１０型～４０型
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
///		漢字データのセット処理
///////////////////////////////////////////////////////////
bool CQRMatrix::AddDataBits_Kanji( const char * pcszData , int iDataLength , int & iErrorType )
{
	bool bRet = true;

	//漢字かどうかのチェック
	for( int i=0 ; i<iDataLength ; i+=2 )
	{
		unsigned short usTemp = (((((unsigned short)pcszData[i]) << 8) & 0xff00) |
									((unsigned short)pcszData[i+1] & 0x00ff) );

		//0x8140～0x9ffc
		if( !(0x8140 <= usTemp && usTemp <= 0x9ffc || 
			  0xe040 <= usTemp && usTemp <= 0xebbf) )
		{
			iErrorType = QRERR_DATAISNOT_KANJI;
			return false;
		}

	}

	//漢字モード
	bRet = AddDataBits( (unsigned char)0x08 , 4 , iErrorType );
	if( !bRet )
	{
		return bRet;
	}

	//文字数の出力
	short sLength = (short)(iDataLength / 2);
	int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;

	//１～９型
	if( iModelNumber <= 9 )
	{
		bRet = AddDataBits( (unsigned char)( sLength & 0xff ) , 8 , iErrorType );
	}
	else
	{
		//１０～２６型
		if( iModelNumber <= 26 )
		{
			bRet = AddDataBits( (unsigned char)((sLength >> 8) & 0x03) , 2 , iErrorType );
		}
		//２７～４０型
		else
		{
			bRet = AddDataBits( (unsigned char)((sLength >> 8) & 0x0f) , 4 , iErrorType );
		}

		if( bRet )
		{
			bRet = AddDataBits( (unsigned char)(sLength & 0xff) , 8 , iErrorType );
		}
	}

	//文字の出力
	for( int i=0 ; i<iDataLength && bRet ; i+=2 )
	{
		unsigned short usCharacter = (((((unsigned short)pcszData[i]) << 8) & 0xff00) |
										((unsigned short)pcszData[i+1] & 0x00ff) );

		//0x8140～0x9ffc
		if( 0x8140 <= usCharacter && usCharacter <= 0x9ffc )
		{
			//0x8140を引く
			usCharacter -= 0x8140;
		}
		//0xe040～0xebbf
		else
		{
			//0xc140を引く
			usCharacter -= 0xc140;
		}
		usCharacter = (((usCharacter >> 8) & 0xff) * 0xc0)	//上位バイトにC0を乗じる
					+  ((usCharacter) & 0xff);				//下位バイトを足す

		//データの出力処理
		bRet = AddDataBits( ((unsigned char)(usCharacter >> 8) & 0x1f) , 5 , iErrorType );
		if( bRet )
		{
			bRet = AddDataBits( (unsigned char)(usCharacter & 0xff) , 8 , iErrorType );
		}
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		データにビットを追加する
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
	//必要バイトサイズ
	int iByteNow = (m_iDataBitNumber + 7) / 8;
	int iByteNeed = (iBitLast + 7) / 8;

	//追加のデータ領域が必要な場合
	if( iByteNeed > m_iDataSizeNumber )
	{
		//128バイト
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

	//今からセットするデータのマスク
	unsigned char ucSetMask = GetBitMask( iBitNumber - 1 );

	//データの追加処理
	for( int i=0 ; i < iBitNumber ; i++ )
	{
		//bitがonの場合
		if( ucSetMask & ucData )
		{
			int iBitToSet = m_iDataBitNumber + i;

			//セット済みデータの次のデータのマスク
			unsigned char ucDataMask = GetBitMask( 7 - (iBitToSet % 8) );

			//該当データのbitもonにする
			int iByteNow = iBitToSet / 8;

			m_pucData[iByteNow] |= ucDataMask;
			
		}
		ucSetMask = (ucSetMask >> 1) & 0x7f;
	}
	
	m_iDataBitNumber += iBitNumber;

	return bRet;
}

///////////////////////////////////////////////////////////
///		２バイトデータを出力する
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
///		指定されたビット番号のビットマスクを取得する
///////////////////////////////////////////////////////////
unsigned char CQRMatrix::GetBitMask( int iBitNumber )
{
	unsigned char ucMask = 0x01 << iBitNumber;

	return ucMask;
}

///////////////////////////////////////////////////////////
///		データのバイト数と誤り訂正レベルから
///		QRの型（1型～40型）を決定する
///////////////////////////////////////////////////////////
bool CQRMatrix::SetTypeNumber(int iCollectLevel , int & iErrorType )
{
	int iByteNum = 0;

	//指定された誤り訂正レベルが不正の場合
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

	//型番号を情報構造体から検索する
	for( int i=0 ; (c_typeMatrix+i)->m_iModelNumber > 0 ; i++ )
	{
		//型によってサイズが異なるため、現在のチェック型に合わせたビットサイズを取得する
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

	//エラーが発生した場合
	if( !bRet )
	{
		iErrorType = QRERR_DATASIZE_LARGE;
	}

	return bRet;
}

///////////////////////////////////////////////////////////
///		誤り訂正コード語の作成処理
///////////////////////////////////////////////////////////
bool CQRMatrix::CreateErrorCollectData( int & iErrorType )
{
	bool bRet = true;

	ReleaseError();

	//誤りデータコード語数（ビット数）
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

	//各ブロックごとにリードソロモンによる符号化を実行する
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
///		データの分割
///		端数が発生する場合、iFirstCountとiSecondCountに分離して
///		整数で分割できるように調整する
///		RSブロックに対するデータ数を計算するのに使用する
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
///		設定された型番号を使用して、モジュール領域を
///		確保し、初期化する
///////////////////////////////////////////////////////////
bool CQRMatrix::InitModuleArea( int & iErrorType )
{
	//型番号からモジュール個数を計算
	m_iModuleNumber = 17 + (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber * 4;

	//モジュールデータの領域確保と初期化
	m_ppucQRMatrix = new char*[m_iModuleNumber];
	//マスクデータの領域確保と初期化
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
///		位置検出パターンのセット
///		位置検出パターンの分離パターンも同時にセットする
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

	//一検出パターン位置
	int iStartPosX[3] = { -1 , m_iModuleNumber - 8 , -1 };
	int iStartPosY[3] = { -1 , -1                  , m_iModuleNumber - 8 };

	for( int i=0 ; i<3 ; i++ )
	{
		//分離パターンの出力
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
///		位置あわせパターンのセット
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

	//型番による位置あわせパターンの縦横方向ループ
	int iModelNumber = (c_typeMatrix + m_iModelTypeIndex)->m_iModelNumber;
	for( int i=0 ; c_sPositionChecks[iModelNumber-1][i] ; i++ )
	{
		for( int j=0 ; j<c_sPositionChecks[iModelNumber-1][j] ; j++ )
		{
			//位置検出パターンとの被りがないかどうかをチェックする
			bool bOutput = true;

			//左上隅
			int x = c_sPositionChecks[iModelNumber-1][i] - 2;
			int y = c_sPositionChecks[iModelNumber-1][j] - 2;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			//右上隅
			x += 4;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			//右下隅
			y += 4;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			//左下隅
			x -= 4;
			if( GetModulePoint( x , y ) != QR_UNKNOWN )
			{
				bOutput = false;
			}

			if( bOutput )
			{
				//各位置あわせパターンの出力用ボックスループ
				for( int k = 0 ; k < 5 ; k++ )
				{
					for( int l = 0 ; l < 5 ; l++ )
					{
						int x = c_sPositionChecks[iModelNumber-1][i] + k - 2;
						int y = c_sPositionChecks[iModelNumber-1][j] + l - 2;

						//黒か白かの判定
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
///		タイミングパターンのセット
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
///		形式情報を書き出す
///		引数：
///			bReserveOnly	：	trueを指定するとデータ領域
///								の予約を行う（すべてLIGHTで塗りつぶす）
///////////////////////////////////////////////////////////
bool CQRMatrix::SetTypeInformation( bool bReserveOnly , int & iErrorType )
{
	//予約のみの場合
	if( bReserveOnly )
	{
		//左上の横方向
		for( int i=0 ; i<9 ; i++ )
		{
			int x = i;
			int y = 8;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		//左上の縦方向
		for( int i = 0 ; i < 8 ; i++ )
		{
			int x = 8;
			int y = i;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		//右上の横方向
		for( int i=0 ; i<8 ; i++ )
		{
			int x = m_iModuleNumber - 8 + i;
			int y = 8;
			if( GetModulePoint( x , y ) == QR_UNKNOWN )
			{
				SetModulePoint( x , y , QR_TYPE_LIGHT );
			}
		}

		//左下の縦方向
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

	//本番出力
	unsigned char ucDataBits[15 + 1] = "000000000000000";

	//エラー訂正符号の取得

	unsigned char ucTypeData = (unsigned char)m_iQRMaskPattern & 0x07;
	ucTypeData |= (((c_typeMatrix + m_iModelTypeIndex)->m_iCollectLevel) << 3) & 0x18;

	//bTypeBitsの頭５桁にセットする
	for( int i=0 ; i<5 ; i++ )
	{
		unsigned char ucMask = GetBitMask( 4 - i );
		ucDataBits[i] = ( (ucMask & ucTypeData) != 0 )?('1'):('0');
	}

	unsigned char ucCoefficients[] = "10100110111";

	bool bRet = false;

	//エラービットを追加する
	bRet = GetExpressionMod( ucDataBits , 5 , ucCoefficients , ucDataBits + 5 , 10 );

	//マスクパターンの適用
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

	//出力処理
	
	//左上横
	int iTemp = 0;
	for( int i = 0 ; i < 8 ; i++,iTemp++ )
	{
		//タイミングパターンに重なる場合
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

	//左上縦
	iTemp = 0;
	for( int i = 0 ; i < 7 ; i++,iTemp++ )
	{
		//タイミングパターンに重なる場合
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

	//左下
	for( int i = 0 ; i < 8 ; i++ )
	{
		int iPointData = QR_TYPE_DARK;
		if( i != 7 && ucDataBits[i] == '0' )
		{
			iPointData = QR_TYPE_LIGHT;
		}

		SetModulePoint( 8 , m_iModuleNumber - i - 1 , iPointData );
	}

	//右上
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
///		ビット多項式の剰余を求める
///		puszCoefficientsの長さはiErrorLength+1となる
///////////////////////////////////////////////////////////
bool CQRMatrix::GetExpressionMod( unsigned char *puszBaseData , int iBaseDataLength,
						   unsigned char *puszCoefficients ,
						   unsigned char *puszError , int iErrorLength )
{
	bool bRet = true;
	int iCoefficientsLength = iErrorLength + 1;

	size_t sizeDataLength = iBaseDataLength + iCoefficientsLength + 1;
	unsigned char * puszData = new unsigned char[sizeDataLength];

	//データ初期化
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
///		型番情報を書き込む
///////////////////////////////////////////////////////////
bool CQRMatrix::SetModelInformation( int & iErrorType )
{
	//型番情報は7型未満の場合（型番情報を記述しない）
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

	//データの出力
	for( int i=0 ; i<6 ; i++ )
	{
		for( int j=0 ; j<3 ; j++ )
		{
			int iPointData = QR_MODEL_DARK;
			if( ucDataBits[(5 - i) * 3 + (2 - j)] == '0' )
			{
				iPointData = QR_MODEL_LIGHT;
			}

			//左側の型番データを出力
			int x = i;
			int y = j + m_iModuleNumber - 11;
			SetModulePoint( x , y , iPointData );

			//右側の型番データを出力
			x = j + m_iModuleNumber - 11;
			y = i;
			SetModulePoint( x , y , iPointData );
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
///		マスクパターンの適応領域の取得
///		現在のモジュール領域内で、QR_UNKNOWNでない部分を
///		マスクパターン適応領域としてセットする
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
///		指定されたインデックスのビットを取得する
///		戻り値：ON：0x01
///				OFF：0x00
///////////////////////////////////////////////////////////
unsigned char CQRMatrix::GetDataBit( int iBitIndex , bool bDataRequest )
{
	int iByteIndex = iBitIndex / 8;

	//データのインデックスが負値の場合
	if( iByteIndex < 0 )
	{
		return (unsigned char)0x00;
	}

	//マスクの取得
	unsigned char ucMask = GetBitMask( 7 - iBitIndex % 8 );

	//データがリクエストされている場合
	if( bDataRequest )
	{
		if( iByteIndex < m_iDataSizeNumber && m_pucData[iByteIndex] & ucMask )
		{
			return (unsigned char)0x01;
		}
	}
	//誤り訂正がリクエストされている場合
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
///		データビットパターンを出力する
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

	//ブロックごとのデータループ
	for( int iBlockIndex=0 ; iBlockIndex<iFirstDataNumber+1 ; iBlockIndex++ )
	{
		//複数ブロックの出力
		for( int iSeq = 0 ; iSeq < iRSBlockNumber ; iSeq++ )
		{
			int iByteIndex = 0;

			//ファーストカウント内の場合
			if( iSeq < iFirstCount )
			{
				iByteIndex = iFirstDataNumber * iSeq;

				//ブロック内インデックスがデータ領域よりも大きい場合
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
	//ブロックごとのデータのループ
	for( int iBlockIndex=0 ; iBlockIndex < iErrorByteNumber ; iBlockIndex++ )
	{
		//ＲＳブロックごとのデータのループ
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
///		次のデータのセットポイントを取得する
/////////////////////////////////////////////////////////////////////
bool CQRMatrix::GetNextDataPutPoint( int & iColumnIndex , int & iRowIndex
									, bool & bFirstColumn
									, bool & bDirectionUp )
{
	//iColumnIndexおよびiRowIndexの位置決め
	while( GetModulePoint( iColumnIndex , iRowIndex ) != QR_UNKNOWN )
	{
		bool bDirectionChanged = false;
		//２列の最初のデータの場合
		if( bFirstColumn )
		{
			iColumnIndex--;
			bFirstColumn = false;
		}
		//縦方向を変更する場合
		else
		{
			//上に移動する場合
			if( bDirectionUp )
			{
				//上の行に
				iRowIndex--;

				//最上段を超えた場合
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
			//下に移動する場合
			else
			{
				//下の行に
				iRowIndex++;

				//最下段を超えた場合
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

			//特殊（列が、タイミングパターン（縦の列の場合）
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
///		マスクパターンの決定
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
		
		//列ごとのチェック
		for( int iRowIndex = 0 ; iRowIndex < m_iModuleNumber ; iRowIndex++ )
		{
			int iSeqBlackModuleNumber = 0;
			int iSeqWhiteModuleNumber = 0;

			for( int iColumnIndex = 0 ; iColumnIndex < m_iModuleNumber ; iColumnIndex++ )
			{
				//暗ポイントの場合
				if( IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
					iSeqBlackModuleNumber++;
					iBlackPointNumber++;

					//連続明ポイントのチェック
					iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
					iSeqWhiteModuleNumber = 0;
				}
				//明ポイントの場合
				else
				{
					iSeqWhiteModuleNumber++;

					//連続暗ポイントのチェック
					iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
					iSeqBlackModuleNumber = 0;
				}

				//２×２ブロックの失点カウント
				iLostPoint += GetLostPoint_ModuleBlock( iColumnIndex , iRowIndex );

				//1：1：3：1：1比率のチェック
				iLostPoint += GetLostPoint_PositionDitect( iColumnIndex , iRowIndex );
			}

			//連続暗ポイントのチェック
			iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
			//連続明ポイントのチェック
			iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
		}

		//暗／明比率
		iLostPoint += GetLostPoint_BlackWhiteRatio( iBlackPointNumber );

		//行ごとのチェック
		for( int iColumnIndex = 0 ; iColumnIndex < m_iModuleNumber ; iColumnIndex++ )
		{
			int iSeqBlackModuleNumber = 0;
			int iSeqWhiteModuleNumber = 0;

			//列ごとのチェック
			for( int iRowIndex = 0 ; iRowIndex < m_iModuleNumber ; iRowIndex++ )
			{
				//暗ポイントの場合
				if( IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
					iSeqBlackModuleNumber++;

					//連続明ポイントのチェック
					iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
					iSeqWhiteModuleNumber = 0;
				}
				//明ポイントの場合
				else
				{
					iSeqWhiteModuleNumber++;
					
					//連続暗ポイントのチェック
					iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
					iSeqBlackModuleNumber = 0;
				}
			}

			//連続暗ポイントのチェック
			iLostPoint += GetLostPoint_SeqModule( iSeqBlackModuleNumber );
			//連続明ポイントのチェック
			iLostPoint += GetLostPoint_SeqModule( iSeqWhiteModuleNumber );
		}

		//一番失点の少ないマスクパターンの取得
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
///		失点のカウント（連続暗・明）
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
///		失点のカウント（指定列・行を開始点とした２×２ブロック）
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
///		失点のカウント（指定位置から1：1：3：1：1の行列チェック
///						マッチする場合には前後の明チェック）
/////////////////////////////////////////////////////////////
int CQRMatrix::GetLostPoint_PositionDitect( int iColumnIndex , int iRowIndex )
{
	bool bDirectionColumn = true;
	int iLostPoint = 0;

	//縦横ループ
	for( int i=0 ; i<2 ; i++,bDirectionColumn = false )
	{
		//調査する方向に７ドット以上のデータが存在しない場合
		if( (bDirectionColumn && iColumnIndex >= m_iModuleNumber - 7) ||
			(!bDirectionColumn && iRowIndex >= m_iModuleNumber - 7 ) )
		{
			continue;
		}

		//1：1：3：1：1のチェック
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

		//1：1：3：1：1が存在しない場合
		if( !bMatched )
		{
			continue;
		}

		//前のスペースをカウントする
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

			//前４つが明の場合
			if( bMatched )
			{
				iLostPoint += 40;
				continue;
			}
		}

		//後ろのスペースをカウントする
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

			//後ろ４つが明の場合
			if( bMatched )
			{
				iLostPoint += 40;
			}
		}

	}
	return iLostPoint;
}

///////////////////////////////////////////////////////////
///		失点のカウント（暗・明比率）
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
