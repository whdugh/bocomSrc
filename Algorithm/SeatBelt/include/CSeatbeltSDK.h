#ifndef __BOCOM_SEATBELT_SDK_H__
#define __BOCOM_SEATBELT_SDK_H__

#include <cv.h>
#include <highgui.h>
#include <fstream>
using namespace std;

class ConvNet;
class Seatbelt
{
public:
	Seatbelt();
	~Seatbelt();

	ConvNet* convnet;

	virtual int InitSeatbeltNet(char* filename);

	int fprop(IplImage *image, CvMat* prob, double prob_temper);

	int DestroySeatbeltNet();


};

class MvSunVisorRecog: public Seatbelt
{
public:
	int InitSeatbeltNet(char* filename);
};

class MvPhoneRecog: public Seatbelt
{
public:
	int InitSeatbeltNet(char* filename);
};
#endif/*__BOCOM_CARCOLOR_SDK_H__*/
