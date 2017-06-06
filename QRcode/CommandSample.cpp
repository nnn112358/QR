
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <iostream>
#include "QRMatrix.h"
using namespace cv;
using namespace std;

//#define MODE_NUMERIC		//�����f�[�^�̏o��
//#define	MODE_ALPHABET		//�A���t�@�x�b�g�f�[�^�̏o��
//#define	MODE_ASCII			//�A�X�L�[������f�[�^�̏o��
//#define	MODE_KANJI			//�����f�[�^�̏o��


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
//	qrMatrix.AddQRCharacters( "�������@�p�q�e�X�g�p�^�[���@������" , QRCHARACTERTYPE_KANJI );
//#endif
//
	int iErrorType;

	//QR�f�[�^�̍쐬
	//�G���[�������x���́i�������j
	//COLLECTLEVEL_L,COLLECTLEVEL_M,COLLECTLEVEL_Q,COLLECTLEVEL_H
	//����I��
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


		//�s�E��̃f�[�^���`�F�b�N���ďo��
		for( int iRowIndex = 0 ; iRowIndex < qrMatrix.GetModuleNumber() ; iRowIndex++ )
		{
			for( int iColumnIndex = 0 ; iColumnIndex < qrMatrix.GetModuleNumber() ; iColumnIndex++ )
			{
				if( qrMatrix.IsDarkPoint( iColumnIndex , iRowIndex ) )
				{
				//	cout << "��";
				qr_image.at<unsigned char>(iColumnIndex, iRowIndex) = 255;
				}
				else
				{
					qr_image.at<unsigned char>(iColumnIndex, iRowIndex) = 0;
				//	cout << "�@";
				}
			}
			//cout << endl;
		}


		// �o�͉摜
		cv::Mat qr_image_out;
		//cv::Mat qr_image_out = cv::Mat::zeros(qr_image.rows * 20, qr_image.cols * 20.0, CV_8U);

		// �摜���T�C�Y
		//cv::resize(qr_image, qr_image_out, qr_image_out.size(), cv::INTER_CUBIC);
		cv::resize(qr_image, qr_image_out, cv::Size(),20, 20);

		threshold(qr_image_out, qr_image_out, 0, 255, THRESH_BINARY | THRESH_OTSU); //臒l�������Őݒ�

		imshow("qr_image_out", qr_image_out);
		imshow("qr_image", qr_image);
		//imwrite("qr_image.tif", qr_image);
		waitKey(0);


	}

	return 0;
}
