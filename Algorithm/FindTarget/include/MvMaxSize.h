#ifndef MV_MAX_SIZE_IMG
#define MV_MAX_SIZE_IMG

#include "Calibration.h"

class MvMaxSize 
{
public:

	MvMaxSize();

	~MvMaxSize();


	void SetMaxSizeImg( FindTargetElePolice::MyCalibration *pCalib, CvSize size);
	
	
	int GetMaxSizeValue(int x, int y) const;


	int GetMaxSizeValue(CvPoint pt) const;

	CvRect GetMaxSizeRect(CvPoint pt) const;

	CvRect GetMaxSizeRect(CvRect rt) const;

	IplImage *GetImgMaxSize() const;

	FindTargetElePolice::MyCalibration *m_pCalib;
private:

	IplImage *imgMaxSize;

	

	MvMaxSize(const MvMaxSize&ms);

	MvMaxSize& operator = (const MvMaxSize&ms);

};

#endif