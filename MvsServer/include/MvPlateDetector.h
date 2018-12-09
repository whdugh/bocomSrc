#pragma once
#include <cv.h>

using namespace std;
using namespace cv;

//调试开关
#define MV_DRAW_DEBUG_LINUX //Linux下建立文件夹开关 
//#define MV_SAVE_RESULT_IMG  //保存结果图片

class MvPlateDetector
{
public:
	MvPlateDetector(void);
	~MvPlateDetector(void);

private:
	CvHaarClassifierCascade * m_pVehicleCascade;	//车辆检测器
	CvHaarClassifierCascade* m_pCascade;			//是否载入分类器

	bool verifySizes(Rect mr);			//判断车牌区域是否合乎要求

public:
	bool mvInit(const char * pName);
	void mvUnInit();

	bool m_bDebugOn;
	//粗定位
	vector<CvRect> mvDetectPos(const IplImage *pImage);

	vector<CvRect> mvDetectPos_Ext(const IplImage *pImage);
	//
	vector<Rect> VerifyPos(const Mat input);
	//卡口调用
	bool IsCarWithPlate(const IplImage * pImg, CvRect rt);
	//违章调用,结合所给区域检测是否是真的车辆，是返回true，不是返回false
	bool JudgeIsRealVehicle(const IplImage * pImg, CvRect rt);
};

