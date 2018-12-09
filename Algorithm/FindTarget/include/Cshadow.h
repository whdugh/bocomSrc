#ifndef CSHADOW_H
#define CSHADOW_H

#define CORRELATHD  0.4
#define WINDWSLIPW  1
#define WINDWSLIPH  1

#include <cv.h>
#include <highgui.h>

class Cshadow
{
public:
    Cshadow();
	virtual ~Cshadow();
    void Init (const IplImage *pBGImg);

	int SclarProduc(const uchar pAvector, const uchar pBVector);
	void DetShadw(const IplImage *pBGImg,const IplImage* pSrcImg,IplImage *pShadowImg);
private:
	IplImage *m_pHSLBg;
	IplImage *m_pHSLSrc;
	bool  m_IntSuc;

};

#endif