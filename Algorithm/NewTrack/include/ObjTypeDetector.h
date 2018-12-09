#ifndef OBJTYPE_DETECTOR_HEADER
#define OBJTYPE_DETECTOR_HEADER

#include <stdio.h>
#include <vector>

#include "libHeader.h"
#include "ImgPro.h"
#include "lsd.h"

#include "hog.h"
#include "MvHoG.h"
#include "MvTypeDetect.h"

#include "comHeader.h"  //放最后

using namespace std;
using namespace MV_IMGPRO_UTILITY;

#ifndef PI_ONE_DEGREE
   #define  PI_ONE_DEGREE  0.0174533
#endif


//#define DEBUG_SHOW_TYPE

class CObjTypeDetector
{
private:
	float m_fScaleSrc2Show;
	CvPoint2D32f m_ptLocPt1;
	CvPoint2D32f m_ptLocPt2;
	CvPoint2D32f m_ptCarSz1;
	CvPoint2D32f m_ptCarSz2;

public:
	void mvGetCarSzConfigInfo( 
			float fSrc2Show,
			CvPoint2D32f pt1,
			CvPoint2D32f pt2,
			CvPoint2D32f ptCarSz1, 
			CvPoint2D32f ptCarSz2
		);
public:
	CObjTypeDetector( );
	virtual ~CObjTypeDetector( );

public:
	CvPoint2D32f mvGetCarSz( 
			CvPoint2D32f pt, 
			CvPoint2D32f pt1, 
			CvPoint2D32f pt2,
			CvPoint2D32f ptCarSz1, 
			CvPoint2D32f ptCarSz2 
		);

	bool mvGetObjType( 
			IplImage *pRgbImg, 
			IplImage *pBgImg,
			IplImage *pFkImg
		);
	bool mvGetSobel( 
			IplImage *pRgbImg,
			IplImage *pBgImg 
		);
 
	bool mvCarJudgeWithLines( 
			vector<int> &vctSuitLineNo,             //OUT:满足的线段序号 
			const vector<CvPoint2D32f> &vctLinePt1, //IN:线段起点
			const vector<CvPoint2D32f> &vctLinePt2, //IN:线段终点
			float fLenThOfCar,                      //IN:车辆上的线段长度要求 
			IplImage *pSubImg = NULL                //IN:用于调试的彩色图像
		); 
	bool mvCarJudgeWithSobel(
			IplImage *pSubImg,
			IplImage *pFkImg, 
			float &fXSobR,
			float &fYSobR
		);
	bool mvCarJudgeWithColor( 
			IplImage *pRgbImg, 
			IplImage *pFkImg,
			float &fRatio
		);

	//判断是否存在行人,并返回其个数和位置
	bool mvIsExistPeople( 
			IplImage *pImg,
			CvPoint maxsizePt, 
			vector<float> &vctPredictVal, 
			vector<CvRect> &vctRectPeople,
			bool bHardHog = true
		);

	//判断是否存在行人,并返回其个数和位置
	bool mvIsExistPeopleWithSimHoG( 
			vector<float> &vctPredictVal,
			vector<CvRect> &vctRectPeople,
			AnStruSimHoGPeoDetApp *pSimHoGPeoDetApp,
			IplImage *pDetImg, 
			CvPoint ptPeoWh,
			bool bHardHog = true
		);

	//利用简单HoG判断图像某区域是否为行人
	bool mvGetPeoWinWithSimplexHoG( 
			float *fPeoDetector, 
			IplImage *&pHogImg, 
			CvSize winStride, 
			float fThres,
			int &nWindow,
			int &nPeoWindow,
			CvRect APeoRect[], 
			float fAPredictScore[]
		);
};

#endif //OBJTYPE_DETECTOR_HEADER