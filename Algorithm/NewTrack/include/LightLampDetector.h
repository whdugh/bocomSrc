//灯光和车灯检测
#ifndef __MV_LIGHT_LAMP_DETECTOR_H
#define __MV_LIGHT_LAMP_DETECTOR_H

#include "libHeader.h"
#include "GlobalDetRes.h"

//车灯检测器
typedef struct StruLampDetector
{
public:
	//该函数实现对给定的灰度图像进行亮区检测
	static bool mvGetBrightRect(
			BrightAreaOnNightRes &brightArea, //检测到的亮区结果
			double dTsNow,             //当前时间戳
			const IplImage *pGrayImg,  //灰度图像
			const IplImage *pCarWImg,  //车宽图像
			const IplImage *pCarHImg   //车高图像
		);

	//对给定区域进行车灯检测
	static bool mvDetectLamp4GiveArea(
			vector<CvRect> &vectLefLamp,  //检测到的左车灯
			vector<CvRect> &vectRigLamp,  //检测到的右车灯
			bool bJustDetOneLampPair,     //只检测一对车灯
			const CvPoint &ptObjLt,  //给定的区域的左上点
			const CvPoint &ptObjRb,  //给定的区域的右下点
			const IplImage *pCarWImg, //车宽图像
			const IplImage *pCarHImg, //车高图像
			const int nBrightAreaCnt,   //亮区数目
			const CvRect *pBrightRects, //各亮区rect
			IplImage *pDrawImgg=NULL  //用于绘制的图像
		);

	//亮区检测 和 对对给定区域进行车灯检测
	static bool mvDetectBrightAreaPairLamp(
			BrightAreaOnNightRes &brightAreaRes, //亮区检测结果
			AnPairLampResult &pairLampRes,       //车灯对检测结果
			double dTsNow,			   //当前时间戳
			bool  bJustDetOneLampPair, //是否一区域只检测一对车灯
			int   nAreaCnt,			  //给定的区域数目
			const int *nAObjIdx,	  //各区域所对应的目标序号(可无)
			const CvPoint *ptAObjLt,  //各区域的左上点
			const CvPoint *ptAObjRb,  //各区域的右下点
			const IplImage *pGrayImg, //灰度图像
			const IplImage *pCarWImg, //车宽图像
			const IplImage *pCarHImg  //车高图像
		);

}AnLampDetector;


//灯光检测器
typedef struct StruLightDetector
{
public:
	//检测车灯光检测的接口
	static bool mvDetectLampLight(  
			GlobalDetRes &gobalDetRes,     //全局的结果保存器
			double dTsNow,                 //当前时间戳
			const IplImage *pGrayImg,      //当前灰度图
			const IplImage *pIntGrayImg,   //当前灰度积分图
			const IplImage *pGrayBgImage,  //当前灰度背景图
			const IplImage *pIntBgGrayImg, //当前灰度背景积分图
			const IplImage *pCarWImg,      //车辆大小图
			const IplImage *pCarHImg,      //车辆大小图
			vector<int> &vectLampObjIdx,   //车灯所对应的目标序号
			const vector<fkContourInfo> &vcetForeContours  //前景轮廓
		);

	//对每个给定的区域进行照射光检测，得到衰减区域信息
	static void mvGetShineLightMsg(
			vector<ShineLightMsg> &vctShineLight, //获取到的灯光衰减区域
			const IplImage *pGraySmoothImg,  //当前灰度的平滑图
			const IplImage *pIntGrayImg,     //当前灰度积分图
			const IplImage *pGrayBgImage,    //当前背景图
			const IplImage *pIntBgGrayImg,   //当前背景积分图
			const IplImage *pCarMaxSizeXImg, //车辆宽度结果图
			const IplImage *pCarMaxSizeYImg, //当前宽度结果图
			const CvRect &rectFk,            //给定的前景区域
			const vector<CvRect> &vctTwoLampRect,   //所有的车灯对区域
			const vector<CvRect> &vctBigBrightRect,  //所有的较大亮区
			IplImage *pDrawLumImg=NULL,      //绘制的灯光检测图像
			bool bT2B=true,				 //是否从上到下检测灯光
			bool bB2T=true				 //是否从下到上检测灯光
		);

	//得到衰减区域的起始点、终止点和最大亮度值
	static void mvGetStartEndPoint(
			const IplImage *pGrayImage,   //当前灰度图
			const IplImage *pGrayBgImage, //背景灰度图
			const CvRect &aRect,	//前景轮廓区域
			int nLineX,				//垂直线段的x坐标
			const vector<CvRect> &vctTwoLampRect,   //车灯对区域
			const vector<CvRect> &vctBigBrightRect, //大亮区区域
			int &nMaxLumVal, //最亮的灰度值
			int &nStartIDY,  //y开始的位置值
			int &nEndIdY,     //y结束的位置值
			int nMod
		);

	//计算理论上直线的斜率
	static float mvComputeKinTheory(
			int nMaxLumVal,      //该区域像素的最大亮度
			int nAreaHeight,     //区域的高度
			int nAGrayHist[256], //区域的灰度直方图 
			int &nMinLumVal      //该区域像素的最小亮度
		);

}AnLightDetector;


//灯光区域判断结构体
typedef struct StructLightAreaJudge
{
public:
	StructLightAreaJudge( )
	{
#ifdef DEBUG_BRIGHT_LAMP_LIGHT_DETECT
		m_dCreateTs = -1000.0;
		m_pDrawRgbImg = NULL;		
#endif
	}

	~StructLightAreaJudge( )
	{
#ifdef DEBUG_BRIGHT_LAMP_LIGHT_DETECT
		if( NULL!=m_pDrawRgbImg )
		{
			cvReleaseImage(&m_pDrawRgbImg);
		}
#endif
	}

	//判断给定的一个区域是否为灯光区域
	bool mvJudgeIsLightArea(
			GlobalDetRes &gobalDetRes,       //全局的检测结果(输入)
			double dTsNow, const IplImage *pGrayImg, //当前时间戳/灰度图
			const CvRect &rectGiveArea,      //给定的判断区域
			const CvPoint &ptStdCarWH		 //区域标准小车宽高
		);

	//判断给定的区域是否为灯光区域
	bool mvJudgeAreLightArea( 
			vector<int> &vectLightDetRes,    //对灯光的检测结果 
			GlobalDetRes &gobalDetRes,       //全局的检测结果(输入)
			double dTsNow, const IplImage *pGrayImg,  //当前时间戳/灰度图
			const vector<CvRect> &rectGiveArea,  //给定的判断区域
			const vector<CvPoint> &ptStdCarWH    //区域标准小车宽高
		);

#ifdef DEBUG_BRIGHT_LAMP_LIGHT_DETECT
	double m_dCreateTs;
	IplImage *m_pDrawRgbImg;  //每次使用前先释放再开辟，在析构时再释放
#endif

private:

#ifdef DEBUG_BRIGHT_LAMP_LIGHT_DETECT
	//绘制当前的亮区/车灯和灯光检测结果
	void mvDrawNowBrightLampLightRes(
			double dTsNow, const IplImage *pNowGrayImg,   //当前的时间戳/灰度图 
			const vector<AnBrightAreaRes> *pVectBrightArea,   //亮区检测结果指针
			const vector<AnLampPairDetRes> *pVectLampPairRes, //车灯对检测结果指针
			const vector<ShineLightMsg> *pVectLampLightRes    //车灯光检测结果指针
		);
#endif

	//判断给定区域是否为灯光区域
	bool mvIsLightArea(
			const CvRect &rectGiveArea,  //给定的判断区域
			const CvSize &szImg,         //对应的图像尺寸
			const CvPoint &ptStdCarWH,   //该区域标准小车的宽高
			const vector<AnBrightAreaRes> *pVectBrightArea,   //亮区检测结果指针
			const vector<AnLampPairDetRes> *pVectLampPairRes, //车灯对检测结果指针
			const vector<ShineLightMsg> *pVectLampLightRes   //车灯光检测结果指针
		);

}AnLightAreaJudge;






#endif