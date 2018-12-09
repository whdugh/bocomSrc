#ifndef __BOCOM_CARLOGO_G_SDK_H__
#define __BOCOM_CARLOGO_G_SDK_H__

#include <cv.h>
#include <highgui.h>
#include <fstream>
using namespace std;

class ConvNet;
class Carlogo_G
{
public:
	Carlogo_G();
	~Carlogo_G();

	ConvNet* convnet;

	int InitCarlogoNet(char* filename);

	int fprop(IplImage *image, CvMat* prob, double prob_temper);

	int DestroyCarlogoNet();
};

#endif
