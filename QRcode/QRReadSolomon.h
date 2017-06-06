#ifndef _QRREADSOLOMON_H
#define	_QRREADSOLOMON_H

//QRコードのエラー訂正で使用するリードソロモンの計算
class CQRReadSolomon
{
public:
	CQRReadSolomon();
	~CQRReadSolomon();

	bool CodeTo( unsigned char * pucData , int iDataLength
				,unsigned char * pucCode , int iCodeLength );

protected:
	struct SAlphaNumber * m_pNumberAlpha;
	static int SortCompareNumberAlpha( const void * pcParam1 , const void * pcParam2 );
	unsigned char * GetExpressionCoefficients( int iCodeLength );

};

#endif
