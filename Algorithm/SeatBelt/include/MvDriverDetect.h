#ifndef MV_DriverDetect_H_
#define MV_DriverDetect_H_

#define DETECT_WINDOW_SHOULDER


#include<opencv2/opencv.hpp>
#include "MvMulObjDetect.h"  //车检库头文件
#include "MvSafetyBeltClassify.h"

using namespace std;
using namespace cv;

#ifndef WINDOW_AREA
#define WINDOW_AREA

struct WindowArea
{
	Target tWindow;

	Target tPilot;			//主驾驶员位置，用于安全带打手机，遮阳板

	bool bPilotByDetect;	//主驾驶是否由检测而来

	Target tCopilot;		//副驾驶员位置，用于安全带，遮阳板

	bool bCopilotByDetect;  //副驾驶是否由检测而来，为是做安全带识别

	bool bAllByGuess;       //主驾和车窗均未检测到，所有位置信息均不可信

	WindowArea()
	{
		tWindow.dConfidece = 0.0;
		tPilot.dConfidece = 0.0;
		tCopilot.dConfidece = 0.0;

		bPilotByDetect = true ;	 //初始化主驾位置由检测而来，而非车窗估计而来

		bCopilotByDetect = true; //初始化副驾位置由检测而来，而非车窗估计而来
		
		bAllByGuess = false;     //若为true，可直接返回不做任何识别 
	}

};

#endif


class MvDriverDetect
{
public:
	MvDriverDetect(void);
	~MvDriverDetect(void);

public:
	bool mvInit( char* );

	bool mvInit( char* chhead = NULL, char* chVisor = NULL, char* chPhone = NULL );

	void mvUnInit();

	int mvWindowShoulderDetect(IplImage* src ,CvRect rROI, WindowArea& tArea,int ncartype );

	vector<Rect> mvMainDriverDetect(IplImage *pImage,const double dBigSideLength=200.00,const double dThres=0.0 );

	vector<Rect> mvBeltDetect(IplImage *pImage,float *fvaulS,const double dBigSideLength=150.00,
		const double dThres = 0.0, int ncartype = 1, int *nsize = NULL);
	
	vector<FaceRt> mvSvmDetect(IplImage *pImage,CvRect position,int ncartype,
		const double dBigSideLength=250.00,const double dThres=0.0 );

	vector<Target> mvVisorDetect(IplImage *pImage, int &nID,const double dBigSideLength=150.00 );

	vector<Rect> mvPhoneDetect(IplImage *pImage, float *fvaulS,const double dBigSideLength=150.00,const double dThres=0.0);

	Rect mvPossibleDriverPos(IplImage* pImg,CvRect position, int ncartype);

	vector<Target> mvWinDetect(IplImage *pImage, int &nID, const double dBigSideLength =200.00 );

private:
	HOGDescriptor* hog_driver;

	HOGDescriptor* hog_Phone;

	//头肩
	MvMulObjDetect *objhead;

	//遮阳板
	MvMulObjDetect *objSunVisor;

	//add by liuyang 新策略，车窗头肩一起检测
	MvMulObjDetect *pWindow_Shoulder;

	Target MvDriverDetect::mvBiggestOne(vector<Target> vTarget);

	int mvRationality( WindowArea& tW, IplImage* pImage );



	double mvRectCoverRate( Rect r1, Rect r2 );   //计算两个矩形的重叠率

	bool mvIsVerticalShift( Rect r1, Rect r2 );  // 水平距离

	Rect mvRectCombineSmall( Rect r1, Rect r2 );     //合并两个矩形 窄策略

	Rect mvRectCombineBig( Rect r1, Rect r2 );     //合并两个矩形 宽策略

	int mvRectGroup( vector<Rect>& V_R, vector<double>& V_W,const double dThres );        // 一个Rect容器合并

	vector<Rect> mvRationality( vector<Rect> V, const IplImage* pImage );		// 合理性判断

	vector<FaceRt> mvFaceRationality( vector<FaceRt> V, const IplImage* pImage );		// 合理性判断

	//void mvDriverDetect(IplImage *pImage,const double dBigSideLength=200.00, Rect& rLRect=Rect(0,0,0,0),Rect& rRRect=Rect(0,0,0,0) );

	FaceRt mvGetHighScoreRect(const vector<Rect> V_R, const vector<double> V_W);

	float mvGetHighScorevaule(const vector<double> V_W);

	int mvEdgeSelect(IplImage *src,IplImage *dst);

};

#endif

