#include <windows.h>

#include "QRMatrix.h"

#define MODE_NUMERIC		//�����f�[�^�̏o��
#define	MODE_ALPHABET		//�A���t�@�x�b�g�f�[�^�̏o��
#define	MODE_ASCII			//�A�X�L�[������f�[�^�̏o��
#define	MODE_KANJI			//�����f�[�^�̏o��

#define QRUNITSIZE	4		//QR�̒P�ʃh�b�g��
#define	QRMARGIN	4		//�}�[�W��

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
///		�p�q�R�[�h�̏o�͏���
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
	qrMatrix.AddQRCharacters( "�������@�p�q�e�X�g�p�^�[���@������" , QRCHARACTERTYPE_KANJI );
#endif

	int iErrorType;

	//QR�f�[�^�̍쐬
	//�G���[�������x���́i�������j
	//COLLECTLEVEL_L,COLLECTLEVEL_M,COLLECTLEVEL_Q,COLLECTLEVEL_H
	//����I��
	if( !qrMatrix.MakeQRMatrix( COLLECTLEVEL_M , iErrorType ) )
	{
		const char * pcszError = "Error Occured!";
		TextOut( hdc , QRUNITSIZE*QRMARGIN , QRUNITSIZE*QRMARGIN , (LPCWSTR)pcszError , (int)strlen(pcszError) );
	}
	else
	{
		//�`��p�̃y���ƃu���V�̍쐬
		HBRUSH hOldBrush , hBrushBack , hBrushFront;
		HPEN   hOldPen   , hPenBack   , hPenFront;

		hPenBack  = CreatePen( PS_SOLID , 1 , RGB( 255 , 255 , 255 ) );
		hPenFront = CreatePen( PS_SOLID , 1 , RGB(   0 ,   0 ,   0 ) );

		hBrushBack  = CreateSolidBrush( RGB( 255 , 255 , 255 ) );
		hBrushFront = CreateSolidBrush( RGB(   0 ,   0 ,   0 ) );

		//�w�i�F�̃Z�b�g
		hOldPen   = (HPEN)SelectObject( hdc , hPenBack );
		hOldBrush = (HBRUSH)SelectObject( hdc , hBrushBack );

		//�w�i�F�̓h��Ԃ�
		Rectangle( hdc , 0 , 0 , (qrMatrix.GetModuleNumber() + QRMARGIN * 2) * QRUNITSIZE
							   , (qrMatrix.GetModuleNumber() + QRMARGIN * 2) * QRUNITSIZE );

		//�O�i�F�̃Z�b�g
		SelectObject( hdc , hPenFront );
		SelectObject( hdc , hBrushFront );

		//���������̂ݓh��Ԃ�
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

		//�㏈��
		SelectObject( hdc , hOldPen );
		SelectObject( hdc , hOldBrush );

		DeleteObject( hPenBack );
		DeleteObject( hPenFront );

		DeleteObject( hBrushBack );
		DeleteObject( hBrushFront );
	}
}
