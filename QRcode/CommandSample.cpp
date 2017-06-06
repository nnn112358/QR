
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <iostream>
#include "QRMatrix.h"
using namespace cv;
using namespace std;

//#define MODE_NUMERIC		//数字データの出力
//#define	MODE_ALPHABET		//アルファベットデータの出力
//#define	MODE_ASCII			//アスキー文字列データの出力
//#define	MODE_KANJI			//漢字データの出力


int main(){

	CQRMatrix qrMatrix;

//#ifdef MODE_NUMERIC
//	qrMatrix.AddQRCharacters( "0123456" , QRCHARACTERTYPE_NUMERIC );
//#endif
//
//#ifdef MODE_ALPHABET
//	qrMatrix.AddQRCharacters( "ABCDEFG" , QRCHARACTERTYPE_ALPHABET );
//#endif

//#ifdef MODE_ASCII
	qrMatrix.AddQRCharacters( "\nhttp://www.google.com\n" , QRCHARACTERTYPE_ASCII );
//#endif

//#ifdef MODE_KANJI
//	qrMatrix.AddQRCharacters( "★☆★　ＱＲテストパターン　★☆★" , QRCHARACTERTYPE_KANJI );
//#endif
//
	int iErrorType;

	//QRデータの作成
	//エラー訂正レベルは（第一引数）
	//COLLECTLEVEL_L,COLLECTLEVEL_M,COLLECTLEVEL_Q,COLLECTLEVEL_H
	//から選択
	if( !qrMatrix.MakeQRMatrix( COLLECTLEVEL_M , iErrorType ) )
	{
		cout << "Error Occured! ErrorType is " << iErrorType << endl;
	}
	else
	{
	//	Mat src5 = Mat::ones(100, 160, CV_8U);

		int ww=qrMatrix.GetModuleNumber();
		int hh=qrMatrix.GetModuleNumber();
		Mat qr_image = Mat::zeros(cv::Size(ww,hh), CV_8UC1);


		//行・列のデータをチェックして出力
		for( int iRowIndex = 0 ; iRowIndex < qrMatrix.GetModuleNumber() ; iRowIndex++ )
		{
			for( int iColumnIndex = 0 ; iColumnIndex < qrMatrix.GetModuleNumber() ; iColumnIndex++ )
			{
				if( qrMatrix.IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
				//	cout << "■";
				qr_image.at<unsigned char>(iColumnIndex, iRowIndex) = 255;
				}
				else
				{
					qr_image.at<unsigned char>(iColumnIndex, iRowIndex) = 0;
				//	cout << "　";
				}
			}
			//cout << endl;
		}


		// 出力画像
		cv::Mat qr_image_out;
		//cv::Mat qr_image_out = cv::Mat::zeros(qr_image.rows * 20, qr_image.cols * 20.0, CV_8U);

		// 画像リサイズ
		//cv::resize(qr_image, qr_image_out, qr_image_out.size(), cv::INTER_CUBIC);
		cv::resize(qr_image, qr_image_out, cv::Size(),20, 20);

		threshold(qr_image_out, qr_image_out, 0, 255, THRESH_BINARY | THRESH_OTSU); //閾値を自動で設定

		imshow("qr_image_out", qr_image_out);
		imshow("qr_image", qr_image);
		//imwrite("qr_image.tif", qr_image);
		waitKey(0);


	}

	return 0;
}
