#ifndef _QRDATAELEMENT_H
#define	_QRDATAELEMENT_H

//ＱＲデータのタイプ
#define	QRCHARACTERTYPE_UNKNOWN		(-1)
#define	QRCHARACTERTYPE_NUMERIC		(1)
#define	QRCHARACTERTYPE_ALPHABET	(2)
#define	QRCHARACTERTYPE_ASCII		(3)
#define	QRCHARACTERTYPE_KANJI		(4)

#define	QRDATA_LAST		(4)

class CQRDataElement
{
	friend class CQRMatrix;
public:
	CQRDataElement();
	CQRDataElement( const CQRDataElement & qrElement );
	~CQRDataElement();

	void SetData( const char * pcszString , int iDataLength, int iDataMode );
	bool AddData( const char * pcszString , int iDataLength, int iDataMode );

protected:
	void InitData();
	void ReleaseData();
	int m_iDataMode;
	int m_iDataLength;
	char * m_pszData;
};

#endif
