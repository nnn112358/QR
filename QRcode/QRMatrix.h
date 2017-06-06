/*
ＱＲ作成手順
１：CQRMatrixのインスタンスを作成する
例）
CQRMatrix qrArray;

２：QRで表現するデータを登録する
例）
qrArray.AddQRAscii( "http://www.spoonsoftware.com" );

３：誤り訂正レベルを指定してQRコードを作成する
例）
qrArray.MakeQRArray( COLLECTLEVEL_M );

４：GetModuleNumberを呼び出し、マトリックスの行列番号を取得する
例）
int iModuleNumber = qrArray.GetModuleNumber();

５：２次元のループを行い、縦横データにアクセスする
例）
for( int i=0 ; i<iModuleNumber ; i++ )
{
	for( int j=0 ; j<iModuleNumber ; j++ )
	{
		if( qrArray.GetModulePoint( j , i ) )
		{
			printf( "■" );
		}
		else
		{
			printf( "　" );
		}
	}
}
*/

#ifndef _QRARRAY_H_
#define	_QRARRAY_H_

#include <list>
#include "QRDataElement.h"

using namespace std;

//誤り訂正レベル
#define	COLLECTLEVEL_L		(0x01)
#define	COLLECTLEVEL_M		(0x00)
#define	COLLECTLEVEL_Q		(0x03)
#define	COLLECTLEVEL_H		(0x02)

//QR作成時、エラー番号
#define	QRERR_NOERROR				(0)
#define	QRERR_WRONG_COLLECTTYPE		(1)
#define QRERR_DATASIZE_LARGE		(2)
#define	QRERR_DONTSET_TYPENUMBER	(3)
#define	QRERR_DATAISNOT_NUMERIC		(4)
#define	QRERR_DATAISNOT_ALPHA		(5)
#define	QRERR_DATAISNOT_KANJI		(6)
#define	QRERR_WRONGDIRECT_BIT		(7)
#define	QRERR_DATABITCOUNT_FAILURE	(8)

//QRコードの各パターン
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

	//データセット系
	bool AddQRCharacters( const char * pcszString , int iCharacterType );

	bool MakeQRMatrix( int iCollectLevel , int & iErrorType );

	///////////////////////////////////////////////////////////
	///		QRコードの型数を取得する
	///		型数は1～40となる
	///////////////////////////////////////////////////////////
	inline
	int GetTypeNumber()
	{
		return m_iModelTypeIndex;
	}

	///////////////////////////////////////////////////////////
	///		モジュール個数を取得する
	///		モジュール個数がマトリックスの縦・横数となる
	///////////////////////////////////////////////////////////
	inline 
	int GetModuleNumber()
	{
		return m_iModuleNumber;
	}

	///////////////////////////////////////////////////////////
	///		指定された行列に対応するポイントデータを取得する
	///		戻り値：　==false：明領域
	///				　==true：暗領域
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

		//未処理領域の場合
		if( m_ppucQRMatrix[iColumnIndex][iRowIndex] < (char)0 )
		{
			iModulePointOrg = 0;
		}
		//処理済み領域の場合
		else
		{
			iModulePointOrg = GetModulePoint( iColumnIndex , iRowIndex ) % 2;
		}

		//マスクがかからないデータの場合
		if( m_ppucMaskMatrix[iColumnIndex][iRowIndex] == QR_UNKNOWN || m_iQRMaskPattern < 0 )
		{
			return (iModulePointOrg !=0 );
		}

		//マスクがかかるデータの場合
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
	///		指定された行列に対応するポイントデータをセットする
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
	///		指定された行列に対応するポイントデータを取得する
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
	///		指定行、列のマスクパターンを取得する
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

		//仕様書がi,jなので、混乱しないように変換
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

	//QRデータのマトリックス本体
	//各要素は以下のとおりとなる。
	//　< 0 ：未処理領域(QR_UNKNOWN)
	//  % 2 == 0 ：明領域
	//  % 2 != 0 ：暗領域
	//
	//　== 0 ：データ領域（明）(QR_DATA_LIGHT)
	//  == 1 ：データ領域（暗）(QR_DATA_DARK)
	//  == 2 ：エラー訂正領域（明）(QR_COLLECT_LIGHT)
	//  == 3 ：エラー訂正領域（暗）(QR_COLLECT_DARK)
	//  == 10：位置検出パターン（明）（分離パターンを含む）(QR_POSITIONDITECT_LIGHT)
	//  == 11：位置検出パターン（暗）（分離パターンを含む）(QR_POSITIONDITECT_DARK)
	//  == 12：位置合わせパターン（明）(QR_POSITIONCHECK_LIGHT)
	//  == 13：位置合わせパターン（暗）(QR_POSITIONCHECK_DARK)
	//  == 14：タイミングパターン（明）(QR_TIMING_LIGHT)
	//  == 15：タイミングパターン（暗）(QR_TIMING_DARK)
	//  == 20：形式情報（明）(QR_TYPE_LIGHT)
	//  == 21：形式情報（暗）(QR_TYPE_DARK)
	//  == 22：型番情報（明）(QR_MODEL_LIGHT)
	//  == 23：型番情報（暗）(QR_MODEL_DARK)
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

