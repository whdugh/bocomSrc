//本模块而实现红灯区域的校正
#include "cv.h"
#include "ml.h"
#include "highgui.h"
#include "cxcore.h"
#include <iostream>
#include <fstream>

using namespace std;

const int nHsize = 37; //H的bin个数//0~360,在opencv中180/10
const int nSsize = 32; //S的bin个数//0~1,在opencv中256/32

//#define testzch  //本地测试
#define handzch  //提交代码
//#define jinan_yellow //济南横向放置，灯架背景为黄色
#define historyTest
//#define expTest //曝光测试
//注意：rRedrect矩形框是基于图像的，不含在图像上下添加的图像信息标记框
#define liuyang //浏阳项目-学完一种颜色就增

//#define MV_LOWLIGHT_OLD   //低照度老方法
#define MV_LOWLIGHT_NEW   //低照度新方法

//#define MV_COLOR_JUDGE_OLD  //锐势相机-老方法
#define MV_COLOR_JUDGE_NEW  //锐势相机-新方法


#ifndef _LightExpHeader
typedef struct  _LightExpHeader
{
	CvRect LightRegion;//整个灯头外接区域的位置
	int nColorFlag;//曝光红绿灯的颜色信息：0-红灯；1-绿灯；2-黄灯; -1-没有灯亮
	CvRect rLightRect;//红灯位置
	CvRect gLightRect;//绿灯位置
	CvRect yLightRect;//黄灯位置

	_LightExpHeader()
	{
		LightRegion = cvRect(0,0,0,0);
		nColorFlag = 0;
		rLightRect = cvRect(0,0,0,0);
		gLightRect = cvRect(0,0,0,0);
		yLightRect = cvRect(0,0,0,0);
	
	}
}LightExpHeader;//灯框的大矩形
#endif



class MvRedLightAdjust
{
public:
	MvRedLightAdjust(CvRect rRedrect[], int nRednum);//红绿灯偏色

	MvRedLightAdjust(LightExpHeader rLightrect[], int nLightnum, int LightInf[]);//红绿灯曝光

	~MvRedLightAdjust();
	//版本号
	char *GetVersion();
	//函数接口
	int mvRedLightAdjust(IplImage *pImage);
	int mvRedLightAdjustNew(IplImage *pImage, int LightInf[]);
	//红绿灯曝光
	int mvLightExposureAdjust(IplImage *srcImg, int LightInf[]);
	int mvLightRedcastAdjust(IplImage *pImage);
	
	//hs学习
	void mvRedHSTrain();
	void mvHSMatSave();

	void mvHSMatRead();
	IplImage *pSaveImg;
	
	//红灯偏色修正
	void mvRedRevise_zch(IplImage *srcImg, CvRect rectSrc);
	void mvLightRevise(IplImage *srcImg, CvRect rectSrc);//新算法，包含了红绿灯头的位置信息
	void mvHSRevise(IplImage *pImgHsv, IplImage *pdIS, IplImage *pdISMask);//dis修正
	void mvHSReviseByMask();//模板修正
	void mvRedReviseSubPix();
	float ThresholdOtsu(IplImage* grayim, CvRect rect, IplImage *mask); //ostu阈值分割
	bool mvColorDis(CvScalar sRed, CvScalar sLargex);
	void mvImgMoment(IplImage *Pimage, CvRect *rect, CvPoint2D32f *centerNow);

	bool mvJudgeValidRect(IplImage *pSrcImg);

	void mvGetMinMaxArea(LightExpHeader LightTotalRect, float *fMin, float *fMax);
	
	//灯区域检测
	void mvLightExpRegionDetect(int nColorFlg, float fMin, float fMax, CvRect rLHRegion);
	//利用Graph分割-新方法
	void mvLightExpRegionDetectNewSegment(int nColorFlg, float fMin, float fMax, CvRect rLHRegion);

	void mvContourAnalysisBound(IplImage *pMsk);

	//灯颜色增强
	void mvLightExpColorRevise(int nColorFlag, IplImage* pHistoryMsk, CvRect rectR);
	void mvLightExpColorReviseNew(int nColorFlag, IplImage* pHistoryMsk, CvRect rectR);
	
	void mvRemoveByInit(IplImage *plightMsk, IplImage *pMsk);

	//红绿灯增强
	int mvLightColorcastAdjust(IplImage *pImage);

	int mvLightColorcastAdjustNew(IplImage *pImage);

	void mvGreenHSTrain();//绿灯学习
	void mvGreenHSMatSave();//绿灯模型
	void mvRedRevise(IplImage *srcImg, CvRect rectSrc, float fTh);//红灯偏色修正
	void mvGreenRevise(IplImage *srcImg, CvRect rectSrc, float fTh);//绿灯	
	void mvContourAnalysis(IplImage *colorMsk, IplImage *srcImg, CvRect rectSrc);//轮廓分析
	void mvComputeDisimage(IplImage *pMsk, int nColor);//距离图像
	void mvColorRevise(IplImage *phls, IplImage *pdis, IplImage *pMsk, int nColor);//颜色增强

	void mvRedGreenRevise(int *rgbPt, IplImage *pHisRed, IplImage *pHisGreen, int &xCenterAxisRect);//红绿灯

	void mvRedGreenReviseNew();

	bool mvContourAnalysisRGY(IplImage *colorMsk, int *rgbPt);

	void mvContourAnalysisBoundaryNew(IplImage *colorMsk);

	void mvContourAnalysisOnly(IplImage *colorMsk, bool flag, int xCenterAxisRect);

	void mvContourAnalysisOnlyNew(IplImage *colorMsk, int nColorPixNum, int nColorFlag);


	void mvHistorySeq(IplImage *pLightMask, IplImage* pHistoryMsk);
	
public:
	CvRect rRedRect[10];//红灯的roi,注意：该矩形框是基于图像的，不含在图像上下添加的图像信息标记框
	int nRedNum;//红灯的roi个数，最大为10个
	
	LightExpHeader LightRect[10];//灯的矩形框，注意：该矩形框是基于图像的，不含在图像上下添加的图像信息标记框
	int LightRectInf[10];//信号
	int nExpLightNum;//图像中曝光红绿灯大区域个数

private:
	
	int64 start;//启动本模块儿的时间
	int64 updateTh;//更新模块的时间间隔，初步定为30分钟
	int64 updateTime;//用于建立新模块的时间，初步设定为2分钟
	int64 TimeLast;//修正阶段，前一次hs的学习结束时间
	
	bool bInit;//初始化是否结束
	bool bInitLast;//前一个颜色模型训练完毕的时间
	bool bLearn;//学习模型
	bool bLearnOK;//学习完毕，保存更新hs
	bool bTest;//hs红灯修正
	bool bisFirst;//是否第一次学习
	
	IplImage *pRedRect; //红灯
	IplImage *pRedMask; //红灯的掩膜
	IplImage *pRedHS; //红灯区的hsv空间图
			
	CvSize sizei;//红灯图像的大小
	CvRect biglightRect;

	float mPDH[nHsize][nSsize];//红灯的条件分布直方图
	float mPDHN[nHsize][nSsize];//非红灯的条件分布直方图
	float mHSPDH[nHsize][nSsize];//红色概率分布

	//int nHTh; 
	int nHThLow; //认为是红的h的阈值
	int nHThHigh;//认为是红的h的阈值
	int nSTH; //认为是红灯的s的阈值
	int nVTH; //认为是红灯的v的阈值
	
	int nHcosnt;//标准红灯的h值
	double dVmean;//红灯光照的均值
	double dVmax;//红灯光照的最大值
	double dSmean;//红灯光照的均值
	double dSmax;//红灯光照的最大值
	float WHratio;//长宽比例
	float WHth;
	float fMaxDis;//距离的最大值
	bool bDisMethod;//是否采用骨架信息

	//
	IplImage *pGreenMask; //红灯的掩膜
	int nGThLow;//绿灯
	int nGThHigh;
	int nGHconst;//标准绿灯的h值
	float mPDHG[nHsize][nSsize];//绿灯的条件分布直方图
	float mPDHNG[nHsize][nSsize];//非绿灯的条件分布直方图
	float mHSPDHG[nHsize][nSsize];//绿色概率分布

	int64 TimeLastG;//修正阶段，前一次hs的学习结束时间

	bool bInitG;//初始化是否结束
	bool bInitLastG;//前一个颜色模型训练完毕的时间
	bool bLearnG;//学习模型
	bool bLearnOKG;//学习完毕，保存更新hs
	bool bTestG;//hs红灯修正
	//bool bIsRedRevise;//
	
	//曝光
	int nHexpConst[2];//曝光增强参数
	CvPoint ptRGY[4];//红绿黄灯的矩形框中心

	//红绿灯同时增强
	int RGYcenter[20][6];//与矩形框同步
	//float fLightArea;//灯的面积


	IplImage *phistoryRed[10];//红灯模板
	IplImage *phistoryGreen[10];//绿灯模板
	int xCenterAxis[10];//中轴的位置（震动会造成偏移）
	int nUpdateRatio;//历史模板的更新率
	int nHistoryInitial;//历史学习样本数

	bool bSaveImg;      //保存低照度中间结果
	IplImage *pexpSave;//中间结果保存
	int expNum;//输入的图像帧号
	char cbufExp[256];	//保存路径

	int nSetRedH;
	int nSetRedS;
	int nSetRedV;

};