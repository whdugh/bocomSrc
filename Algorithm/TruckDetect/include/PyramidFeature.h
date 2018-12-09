#ifndef PYRAMID_H_
#define PYRAMID_H_

//#include <opencv.hpp>
//#include "IOG.h"
//#include "DSIFT.h"

class opencv;

class CPyramid
{

public:
	CPyramid();
	~CPyramid();


	//CvMat* m_DSIFT_Mat;
	//CvMat* m_Dict_Mat;
	CvMat* m_Index_Mat;
	CvMat* m_Delta_Mat;
	CvMat* m_EuclideanD_Mat;
	CvMat* m_Mask_Mat;
	IplImage* m_Index_IPL;

	CvScalar m_Dev;

	//CvMat* m_Pyramid_Mat;

	int m_num_layer;
	int m_ScrHeight;
	int m_ScrWidth;
	int m_GridStep;
	int m_Grid_Width;
	int m_Grid_Height;
	int m_num_words;
	int m_PatchSize;

	int init(int ScrHeight, int ScrWidth, int GridStep, int num_word);  

	int generateIndex(CvMat* Dict_Mat, CvMat* DSIFT_Mat);

	int getLengthofPyramid(int num_layar);

	int getPyramid(CvRect bound, CvMat* Pyramid_Mat);

	int clean();


};

#endif