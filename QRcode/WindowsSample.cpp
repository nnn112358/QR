#include <windows.h>

#include "QRMatrix.h"

#define MODE_NUMERIC		//数字データの出力
#define	MODE_ALPHABET		//アルファベットデータの出力
#define	MODE_ASCII			//アスキー文字列データの出力
#define	MODE_KANJI			//漢字データの出力

#define QRUNITSIZE	4		//QRの単位ドット数
#define	QRMARGIN	4		//マージン

static LRESULT CALLBACK WindowProc( HWND , UINT , WPARAM , LPARAM );
static void WriteQR( HDC hdc );

char szWinName[] = "QRSample";

int WINAPI WinMain( HINSTANCE hThisInst , HINSTANCE hPrevInst
				   , LPSTR lpszArgs , int nWinMode )
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wcl;

	wcl.hInstance = hThisInst;
	wcl.lpszClassName = (LPCWSTR)szWinName;
	wcl.lpfnWndProc = WindowProc;
	wcl.style = 0;
	wcl.cbSize = sizeof( WNDCLASSEX );
	wcl.hIcon = LoadIcon( NULL , IDI_APPLICATION );
	wcl.hIconSm = LoadIcon( NULL , IDI_WINLOGO );
	wcl.hCursor = LoadCursor( NULL , IDC_ARROW );
	wcl.lpszMenuName = NULL;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hbrBackground = ( HBRUSH ) GetStockObject( WHITE_BRUSH );

	if( !RegisterClassEx( &wcl ) ) return 0;



	hwnd = CreateWindow((LPCWSTR)szWinName , (LPCWSTR)"QRSample"
						, WS_OVERLAPPEDWINDOW
						, CW_USEDEFAULT	, CW_USEDEFAULT	, CW_USEDEFAULT	, CW_USEDEFAULT
						, HWND_DESKTOP , NULL , hThisInst , NULL );

	ShowWindow( hwnd , nWinMode );
	UpdateWindow( hwnd );

	while( GetMessage( &msg , NULL , 0 , 0 ) ){
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc( HWND hwnd , UINT message , WPARAM wParam , LPARAM lParam )
{
	switch( message ){
	case WM_DESTROY :
		PostQuitMessage( 0 );
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC hdc = BeginPaint( hwnd , &paintStruct );
			WriteQR( hdc );
			EndPaint( hwnd , &paintStruct );
		}
		break;
	default :
		return DefWindowProc( hwnd , message , wParam , lParam );
	}
	return 0;
}

///////////////////////////////////////////////////////////
///		ＱＲコードの出力処理
///////////////////////////////////////////////////////////
void WriteQR( HDC hdc )
{
	CQRMatrix qrMatrix;

#ifdef MODE_NUMERIC
	qrMatrix.AddQRCharacters( "0123456" , QRCHARACTERTYPE_NUMERIC );
#endif

#ifdef MODE_ALPHABET
	qrMatrix.AddQRCharacters( "ABCDEFG" , QRCHARACTERTYPE_ALPHABET );
#endif

#ifdef MODE_ASCII
	qrMatrix.AddQRCharacters( "\nhttp://www.spoonsoftware.com\n" , QRCHARACTERTYPE_ASCII );
#endif

#ifdef MODE_KANJI
	qrMatrix.AddQRCharacters( "★☆★　ＱＲテストパターン　★☆★" , QRCHARACTERTYPE_KANJI );
#endif

	int iErrorType;

	//QRデータの作成
	//エラー訂正レベルは（第一引数）
	//COLLECTLEVEL_L,COLLECTLEVEL_M,COLLECTLEVEL_Q,COLLECTLEVEL_H
	//から選択
	if( !qrMatrix.MakeQRMatrix( COLLECTLEVEL_M , iErrorType ) )
	{
		const char * pcszError = "Error Occured!";
		TextOut( hdc , QRUNITSIZE*QRMARGIN , QRUNITSIZE*QRMARGIN , (LPCWSTR)pcszError , (int)strlen(pcszError) );
	}
	else
	{
		//描画用のペンとブラシの作成
		HBRUSH hOldBrush , hBrushBack , hBrushFront;
		HPEN   hOldPen   , hPenBack   , hPenFront;

		hPenBack  = CreatePen( PS_SOLID , 1 , RGB( 255 , 255 , 255 ) );
		hPenFront = CreatePen( PS_SOLID , 1 , RGB(   0 ,   0 ,   0 ) );

		hBrushBack  = CreateSolidBrush( RGB( 255 , 255 , 255 ) );
		hBrushFront = CreateSolidBrush( RGB(   0 ,   0 ,   0 ) );

		//背景色のセット
		hOldPen   = (HPEN)SelectObject( hdc , hPenBack );
		hOldBrush = (HBRUSH)SelectObject( hdc , hBrushBack );

		//背景色の塗りつぶし
		Rectangle( hdc , 0 , 0 , (qrMatrix.GetModuleNumber() + QRMARGIN * 2) * QRUNITSIZE
							   , (qrMatrix.GetModuleNumber() + QRMARGIN * 2) * QRUNITSIZE );

		//前景色のセット
		SelectObject( hdc , hPenFront );
		SelectObject( hdc , hBrushFront );

		//黒い部分のみ塗りつぶす
		for( int iRowIndex = 0 ; iRowIndex < qrMatrix.GetModuleNumber() ; iRowIndex++ )
		{
			for( int iColumnIndex = 0 ; iColumnIndex < qrMatrix.GetModuleNumber() ; iColumnIndex++ )
			{
				if( qrMatrix.IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
					Rectangle( hdc , (QRMARGIN + iColumnIndex) * QRUNITSIZE
								   , (QRMARGIN + iRowIndex   ) * QRUNITSIZE
								   , (QRMARGIN + iColumnIndex + 1) * QRUNITSIZE
								   , (QRMARGIN + iRowIndex    + 1) * QRUNITSIZE );
				}
			}
		}

		//後処理
		SelectObject( hdc , hOldPen );
		SelectObject( hdc , hOldBrush );

		DeleteObject( hPenBack );
		DeleteObject( hPenFront );

		DeleteObject( hBrushBack );
		DeleteObject( hBrushFront );
	}
}
