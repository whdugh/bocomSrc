#ifndef __BOCOM_CARCOLOR_SDK_H__
#define __BOCOM_CARCOLOR_SDK_H__

#include <cv.h>
#include <highgui.h>
#include <fstream>
using namespace std;
//#include "ConvNet.h"

class ConvNet;
class Carcolor
{
public:
	Carcolor();
	~Carcolor();

	ConvNet* convnet;

	int InitCarcolorNet(char* filename);

	int fprop(IplImage *image, CvMat* prob, double prob_temper);

	int DestroyCarcolorNet();
};

#endif/*__BOCOM_CARCOLOR_SDK_H__*/
