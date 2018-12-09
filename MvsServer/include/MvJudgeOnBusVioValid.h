#pragma once
#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

class MvJudgeOnBusVioValid
{
public:
	MvJudgeOnBusVioValid(void);
	~MvJudgeOnBusVioValid(void);

private:
	CvHaarClassifierCascade* m_pPlateCascade;			//是否载入分类器

	bool verifySizesRt(CvRect rt);			            //判断车牌区域是否合乎要求

public:
	bool mvOnBusVioValidInit(const char *pName);
	void mvOnBusVioValidUnInit();

	//粗定位
	vector<CvRect> mvDetectPlatePos(const IplImage *pImage);

	//判断已有车牌rt右边是否还有车牌
	bool IsHaveCarInPlateRight(const IplImage *pImg, vector<CvRect> &vecRt, CvRect rt);

	//判断在图像右半边是否有车牌
	bool IsHavePlateInImgRight(const IplImage *pImg, vector<CvRect> &vecRt);

	bool mvOnBusVioValidJudge(const IplImage *pImage1,vector<CvRect> &vecRt1,
		                	  const IplImage *pImage2,vector<CvRect> &vecRt2,
		                      const IplImage *pImage3,vector<CvRect> &vecRt3,
							  CvRect plateRt);
};