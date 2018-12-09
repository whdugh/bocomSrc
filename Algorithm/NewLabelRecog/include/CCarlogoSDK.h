#ifndef __BOCOM_CARLOGO_SDK_H__
#define __BOCOM_CARLOGO_SDK_H__

#include <cv.h>
#include <highgui.h>
#include <fstream>
using namespace std;
//#include "ConvNet.h"

class ConvNet;
class Carlogo
{
public:
	Carlogo();
	~Carlogo();

	ConvNet* convnet;

	int InitCarlogoNet(char* filename);

	int fprop(IplImage *image, CvMat* prob, double prob_temper);

	int DestroyCarlogoNet();
};

#endif/*__BOCOM_CARLOGO_SDK_H__*/
