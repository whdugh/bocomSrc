/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvSafetyBeltClassify.h
* 摘要: 简要描述本文件的内容
* 版本: V1.9
* 作者: 俞喆俊、刘洋 
* 完成日期: 2014年12月01日
*/

#ifndef MVSAFETYBELTCLASSIFY__H__
#define MVSAFETYBELTCLASSIFY__H__

#include <cv.h>
#include <highgui.h>
#include "ml.h"
#include <algorithm>
#include <vector>
#include "cvaux.h" 
#include "cxcore.h" 

//--------------控制宏的定义------------ 
//#define MV_SHOWIMG //
#define MV_SAVEIMG_DEBUG
//#define MV_PHONE_TWICE
//#define MV_MULTTHREAD
//如果关掉下句，就没有任何debug记录
//#define  DEBUG_TYPE_ON

using namespace std;
using namespace cv;

struct CARNUM_CONTEXT;
struct Target;

class MvImgEnhance_HandOver;
class Seatbelt;
class Phone;
class MvDriverDetect;

#ifndef OBJECTFACERT
#define OBJECTFACERT
typedef struct stFaceRt
{
	cv::Rect VehicleRect;		//驾驶员区域
	double Weight;			//驾驶员置信度
	int label_type;			//安全带判断
	float fCLP;				//安全带置信度
	float fblur;			//图像模糊度
	int nPhone_type;		//手机判断
	float fPhone_CLP;		//手机置信度
	int nSunVisor_type;		//遮阳板判断
	float fSunVisor_CLP;	//遮阳板置信度
	bool bguess;			

	stFaceRt()
	{
		VehicleRect = cv::Rect(0,0,0,0);
		Weight = 0.0;
		label_type = 0;
		fCLP = 0.0;
		fblur = 0.0;
		nPhone_type = 0;
		fPhone_CLP = 0.0;
		nSunVisor_type = 0;
		fSunVisor_CLP = 0.0;
		bguess = false;
	}
}FaceRt;
#endif

#ifndef OBJECTCAR
#define OBJECTCAR
typedef struct CAR_INFO   //车辆数据结构体
{
	CvRect  plate_rect; //车牌位置,相对于原图
	CvRect  car_rect; //车辆位置,相对于原图
	int plate_color; //车牌颜色（1蓝 2黑 3黄 4白 5其他）
	int carnumrow; //单双牌（1单牌 2双牌 3其他）
	int nCarNumDirection; //车牌的方向(0前牌 1尾牌 2未知)
	char carnum[7]; //车牌号码
	CAR_INFO()
	{
		plate_rect = cvRect(0,0,0,0);
		car_rect = cvRect(0,0,0,0);
		plate_color = 5;
		carnumrow = 3;
		nCarNumDirection = 0;
		memset(carnum,0,sizeof(char)*7);
	}
} car_info;
#endif

class MvSafetyBeltClassify  
{
public:
	
	MvSafetyBeltClassify();
	virtual ~MvSafetyBeltClassify();
	
	//=================================================================
	//功  能: 安全带模块初始化
	//参  数: strPath为模型路径
	//返回值: 错误码
	//=================================================================
	int SafetyBelt_Init( char *strPath = NULL );//初始化




	//=================================================================
	//功  能: 安全带模块主函数，包含（安全带检测，打手机，遮阳板）
	//参  数: pImage：输入需要处理的图片
	//        vehicle_info:输入车牌信息
	//        isDayByTime：输入白天为1,夜晚为0，默认输入1
	//        vecP：输出车内人员区域
	//        ncartype：输入车型信息
	//        bIsSafetyBelt：主驾驶安全带检测模块开关
	//        bIsAideBelt：副驾驶安全带检测模块开关  
	//        bIsPhone：打手机检测模块开关
	//        bIsSunVisor：遮阳板检测模块开关
	//返回值: 是否系安全带，是否打手机，是否有遮阳板
	//=================================================================
	int mvSafetyBelt(IplImage *pImage,const CARNUM_CONTEXT *vehicle_info,int isDayByTime
		,vector<FaceRt> &vecP, bool bIsSafetyBelt, int ncartype, bool bIsAideBelt = false,
		bool bIsPhone = false, bool bIsSunVisor = false);
	
	
	//=================================================================
	//功  能: 安全带模块版本号获取
	//参  数: 无
	//返回值: 错误码
	//=================================================================
	static char* GetVersion();//安全带模块版本


	//=================================================================
	//功  能: 遮阳板检测主函数
	//参  数: pImage：输入图片
	//        rtF：输入驾驶员区域
	//        fCLP：输出遮阳板检测置信度
	//        nIsAideSunVisor：遮阳板检测开关，0为主驾驶，1为副驾驶
	//返回值: 错误码
	//=================================================================
	int mvFindSunVisor(IplImage *pImage, CvRect rtF, float *fCLP, int nIsAideSunVisor);//遮阳板


	//=================================================================
	//功  能: 打手机检测主函数
	//参  数: pImage：输入图片
	//        rtF：输入驾驶员区域
	//        fCLP：输出遮阳板检测置信度
	//        nIsAidePhone：打手机检测开关，0为检测主驾打手机，1为检测副驾驶打手机
	//返回值: 错误码
	//=================================================================
	int mvPhoneDL(IplImage *pImage, CvRect rtF, float *fCLP, int nIsAidePhone);//打手机


	//=================================================================
	//功  能: 安全带检测主函数
	//参  数: pImage：输入图片
	//        rtF：输入驾驶员区域
	//        fACLP：输出安全带检测置信度
	//        nIsAideBelt：安全带检测开关，0为检测主驾安全带，1为检测副驾驶安全带
	//        *fblur：图像模糊度
	//返回值: 错误码
	//=================================================================
	int mvIsSafetyBelt(IplImage *pImage, CvRect rtF, int nIsAideBelt, float *fACLP, //安全带
		float *fblur, int ncartype);


	//=================================================================
	//功  能: 安全带模块释放函数
	//参  数: 无
	//返回值: 错误码
	//=================================================================
	void SafetyBelt_Destroy();//释放



	//=================================================================
	//功  能: 创建输入图像兴趣区域函数
	//参  数: src：输入图片
	//        Brand:车牌位置
	//        m_RectROI：输出的ROI区域
	//        ncartype：输入的车型信息
	//返回值: ROI区域
	//=================================================================
	IplImage* mvCreateROI(IplImage *src, CvRect Brand, CvRect &m_RectROI, int ncartype);


	MvDriverDetect *PeopleDet;

private:

	//=================================================================
	//功  能: 计算图像模糊度函数
	//参  数: pbeltImage：输入图片        
	//返回值: 模糊度
	//=================================================================
	float mvblur(IplImage* pbeltImage);//图像模糊度计算


	//=================================================================
	//功  能: 判断输入图片是正驾驶还是副驾驶
	//参  数: src：输入图片
	//        nmul:输入参数，默认为1
	//返回值: 模糊度
	//=================================================================
	IplImage* Transpolt(IplImage *src, float nmul);//判断正副驾驶


	//=================================================================
	//功  能: 创建输入图像兴趣区域函数
	//参  数: pImage：输入图片
	//        rt：车牌位置
	//        bAllGuess：输出的驾驶员检测结果，是否全是猜的结果
	//        ncartype：输入的车型信息
	//返回值: ROI区域
	//=================================================================
	vector<FaceRt> mvHeadDetect(IplImage *pImage, CvRect rt, int ncartype,bool& bAllGuess);//驾驶员检测

private:
	char buffer_path[512];
	
	
	int first;

	Seatbelt *net_Belt;

	Seatbelt *net_SunVisor;

	Seatbelt *net_Phone;
	
	MvImgEnhance_HandOver *mvEnhance;
};

#endif 
