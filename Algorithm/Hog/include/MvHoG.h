/************************************************************************\
*  HOG (Histogram-of-Oriented-Gradients) Descriptor and Object Detector  *
\************************************************************************/
#ifndef __MV_HOG_H
#define __MV_HOG_H

#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "HoGMacro.h"

#ifdef LINUX	
	#include <sys/time.h>
	#include <sys/timeb.h>
	#include <sys/vfs.h>
	#include <sys/statvfs.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#ifndef CHDIR
		#define CHDIR
		#define _chdir(a) chdir(a)
	#endif
	#ifndef MKDIR
		#define MKDIR
		#define _mkdir(a) mkdir(a, 0777)
	#endif
	#ifndef ACCESS
		#define ACCESS
		#define _access(a, b) access(a, b)
	#endif
#else
	#include <direct.h>
	#include <io.h>
#endif

using namespace std;


#define USE_MODIFY_HOG_FEAT  //使用修改后的HoG特征


//简单HoG的检测结果显示模式
#ifndef SIMPLE_HOG_DET_SHOW_MOD
	#define  SIMPLE_HOG_DET_SHOW_MOD	
	enum{	
		NO_SHOW_SIMHOG = 0,		   //不显示
		JUST_SHOW_BESTPEO_SIMHOG,  //显示最好的行人检测样本
		SHOW_DETEPEO_SIMHOG,       //显示检测到的行人检测样本
	};
#endif


//简单HoG的检测结果保存模式
#ifndef SIMPLE_HOG_DET_SAVE_MOD
	#define  SIMPLE_HOG_DET_SAVE_MOD
	enum{	
		NO_SAVE_SIMHOG = 0,		   //不保存
		JUST_SAVE_BESTPEO_SIMHOG,  //保存最好的行人检测样本
		SAVE_DETEPEO_SIMHOG,       //保存检测到的行人检测样本
	};
#endif


class MvSimHOGDesc
{
public:
    enum { 
		L2Hys = 0
	};


	//构造HoG
	void MvConstructSimHOGDesc( 
		int	   _nMaxSlibeWinCnt = -1,   //最大滑动窗口数
		CvSize _winSize = cvSize(SH_WIN_SIZE_X, SH_WIN_SIZE_Y), //模型对应的窗口大小
		CvSize _blockSize = cvSize(BLOCK_SIZE_X, BLOCK_SIZE_Y),  //HoG块的大小
		CvSize _blockStride = cvSize(SH_BLOCK_STRIDEX, SH_BLOCK_STRIDEY), //HoG块滑动的x,y步长
		CvSize _cellSize = cvSize(CELL_SIZE_X, CELL_SIZE_Y),  //HoG块内的单元的大小
		int	   _nBin = GRADDIR_HIST_BIN_CNT, 	//梯度方向数
		double _winSigma = -1,          //窗口的sigma值,用于求高斯权重
		int _histogramNormType = L2Hys,	//直方图归一化模式
		double _L2HysThreshold = 0.2,   //归一化的截断阈值
		bool _gammaCorrection = true	//采用gamma矫正计算梯度	
		 )     
	{
		m_nMaxSlibeWinCnt = _nMaxSlibeWinCnt;    //最大滑动窗口数

		m_winSize = _winSize;
		m_blockSize = _blockSize;
		m_blockStride = _blockStride;
		m_cellSize = _cellSize;

		m_nbins  = _nBin;
		m_nNewBins = m_nbins+1;  //默认加1bin来存梯度较小点的特征

		m_winSigma = _winSigma;  //窗口的sigma值,用于求高斯权重
		m_histogramNormType = _histogramNormType;  //直方图归一化模式
		m_L2HysThreshold =	_L2HysThreshold;      //归一化的截断阈值
		m_bGammaCorrection = _gammaCorrection,	  //采用gamma矫正计算梯度	

		m_bVailidHOG = false;	//当前的HoG模型还不可用
	}


   virtual ~MvSimHOGDesc() { }
 

	//加载的HoG模型
	virtual bool mvLoad( 
			const string& filename,    //欲加载的HoG模型文件
			const int& nModelFeatDim,  //欲加载的HoG模型维数
			vector<float>& detector,   //加载后的HoG模型	
			const int& nMaxSlibeWinCnt,//SimHoG所支持的最大滑动窗口数
			const bool& bUseLineHog=false ); //是否为线段HoG
	

	//对给定图像计算其各窗口的Hog特征值
	virtual int mvComputeHogFeature(
			const IplImage* img,    //给定的图像(不支持ROI)
			float fADescriptors[],  //保留各窗口计算出的HoG特征值
			CvPoint windowLtPt[],   //窗口的左上角点位置
			CvSize winStride=cvSize(0,0),    //窗口滑动的步长
			CvSize padding=cvSize(0,0) ) const;  //留白的大小

	//对给定图像计算其各窗口的Hog特征值并只保存满足为人的
	int mvComputeHogFeatureAndJustSaveSuitPeo( 
			int   &nSuitPeoWinCnt,  //满足为行人的窗口数目
			float fADescriptors[],  //保留各窗口计算出的HoG特征值
			CvPoint ptAWindowLt[],  //窗口的左上角点位置	
			float   fAHoGScore[],   //窗口的HoG预测得分		 
			const IplImage* img,    //给定的图像(不支持ROI)
			int   nHoGDetDim,       //HoG检测器的维数
			float fAHoGDetWight[],  //HoG检测器的各维权重
			float fPeoThres,        //满足为行人的阈值
			CvSize winStride=cvSize(0,0),       //窗口滑动的步长
			CvSize padding=cvSize(0,0) ) const;  //留白的大小

	//获得窗口的sigma系数
	double mvGetWinSigma( ) const;

	//图像梯度结果的计算(新的方法和准则)
	virtual void mvComputeGradientNew(	
			const IplImage* img, //待检测图像(未扩展图的边界)
			CvMat* grad,		 //存图的梯度幅值(为扩展后大小)2通道
			CvMat* angleOfs,     //存图的梯度方向(为扩展后大小)2通道
			CvSize gradSize=cvSize(0,0),    //边界扩展后的大小
			CvSize paddingTL=cvSize(0,0),   //对待检测图左上角扩展的大小
			CvSize paddingBR=cvSize(0,0) ); //对待检测图右下角扩展的大小

private:
	//计算模型的维数
    size_t mvGetDescriptorSize() const;

	//检查加载的HoG的维数是否正确	
    bool mvCheckDetectorSize( bool bUseLineHog=false ) const;

	//得到扩展图中各点在原图里插值
	int mvBorderInterpolate( int p, int len, int borderType );

	//分别得到给定图像x,y方向的窗口个数
	CvSize windowsInImage( CvSize imageSize, CvSize winStride ) const;

	//梯度计算的优化---只支持灰度图
	void mvComputeGradientNewGray(	
			const IplImage* img, //待检测图像(未扩展图的边界)
			CvMat* grad,		//存图的梯度幅值(为扩展后大小)2通道
			CvMat* angleOfs,    //存图的梯度方向(为扩展后大小)2通道
			CvSize gradSize=cvSize(0,0),    //边界扩展后的大小
			CvSize paddingTL=cvSize(0,0),   //对待检测图左上角扩展的大小
			CvSize paddingBR=cvSize(0,0) ); //对待检测图右下角扩展的大小

	//梯度计算的优化---只支持彩色图
	void mvComputeGradientNewRGB(
			const IplImage* img,  //待检测图像(未扩展图的边界)
			CvMat* grad,		  //存图的梯度幅值(为扩展后大小)2通道
			CvMat* qangle,		  //存图的梯度方向(为扩展后大小)2通道
			CvSize gradSize=cvSize(0,0),    //边界扩展后的大小
			CvSize paddingTL=cvSize(0,0),   //对待检测图左上角扩展的大小
			CvSize paddingBR=cvSize(0,0) ); //对待检测图右下角扩展的大小

public:
    CvSize m_winSize;
    CvSize m_blockSize;
    CvSize m_blockStride;
    CvSize m_cellSize;
    int	   m_nbins,  m_nNewBins;
    double m_winSigma;
    int	   m_histogramNormType;   //归一化的方式
    double m_L2HysThreshold;
    bool   m_bGammaCorrection;

    bool m_bVailidHOG;
	vector<float> m_svmDetector;

private:
	int m_nMaxSlibeWinCnt;  //最多的滑动窗口数目

};



//---------------------日期的结构体---------------------//
typedef struct _AN_TYPE_DateStruct
{
public:
	int nNowYear;
	int nNowMon;
	int nNowDay;
	int nNowHour;
	int nNowMin;
	int nNowSec;

	_AN_TYPE_DateStruct( );	

	void getNowDateTime( );

}AnTypeDateStruct;


//------------定义使用HoG来检测人的结构体------------//
typedef struct MvStruHoGPeoApp
{
public:
	MvStruHoGPeoApp( )
	{
		mvInitStruHOGPeoAppVar( );
	};

	void mvInitStruHOGPeoAppVar( );

	//初始化HOG行人检测应用结构体
	bool mvInitStruHOGPeoApp( 
		int nHoGModelType, //所采用的HoG模型类型
		const char chHoGFileName[],  //HOG模型的名称			
		const int &nMaxSlideWinCnt,  //SimHoG所支持的最大滑动窗口数
		const bool &bSaveResult=false,  //是否保存结果      
		const char *chSaveDir=NULL,		//保存结果的目录
		const bool &bUseLineHog=false   //是否使用线段HOG模型
		);
	
	//释放HOG行人检测应用结构体
	bool mvUninitStruHOGPeoApp( );

	//利用HoG检测来获取行人检测结果,返回最好结果的得分和位置
	bool mvGetPeoDetResultWithHoG(
		IplImage *pImg,				 //给定的检测图像
		CvPoint ptLt, CvPoint ptRb,  //给定的检测区域
		int    nResCnt,              //对行人模板缩放的次数
		float  fMaxResScale,         //最大的缩放比率1.2
		float  fResizeRate,			 //每次的缩放比率1.2
		float  fPeoThres,            //行人判断的阈值 0.0
		int    nPeoCntTh,            //行人判断的个数阈值 1
		const CvPoint ptPeoSz,       //检测图区域的行人大小
		vector<float> &vctBestPeoScore,		//检测到的最好行人得分
		vector<CvRect> &vctrectBestPeo,		//检测到的最好行人rect
		IplImage *pShowPeoDetImg=NULL, //显示检测结果
		int  nShowMod=NO_SHOW_SIMHOG, //图像显示的模式
		int  nSaveMod=NO_SAVE_SIMHOG  //图像保存的模式
		);

private:
	//从给出的行人中，挑选得到最好的行人结果
	void mvGetBestPeoples(
		vector<float> *pVctPeoScore,  //检测到行人得分
		vector<CvRect> *pVctPeoRect,  //检测到行人rect
		vector<int> &vctBestPeoIdx   //检测到最好行人序号
		);

private:
	bool m_bInit; //是否完成初始化

private:
	//计算行人HoG特征得分值
	float mvGetPredictValueOfHoGFeat( 
		float *fAHogDescs //给定的行人HoG特征
		);

	//利用简单HoG判断图像某区域是否为行人
	bool  mvGetPeoWinWithSimplexHoG(
		const IplImage *pHogImg, //给定的待检测图像
		CvSize szWinStride,      //窗口滑动的尺度
		const float fPeoThres,   //判断为行人的得分阈值
		int &nPeoWinsNum,        //判断为行人窗口的数目
		int nAWindowIdx[],       //所对应的窗口序号
		CvRect rectAPeo[],		 //各行人rect 
		float fAPeoScore[]   	 //各行人的得分
	);
	//判断是否存在行人,并返回其个数和位置
	bool mvisExistPeopleWithSimpleHoG(
		IplImage *pImg,				 //给定的检测图像
		CvPoint ptLt, CvPoint ptRb,  //在检测图像的区域
		int    nResCnt,				 //对行人模板缩放的次数
		float  fResizeScale[],       //对行人模板缩放比率 1.25, 1.0, 0.8,
		float  fPeoThres,            //行人判断的阈值 0.0
		const CvPoint ptPeoSz,       //检测图区域的行人大小
		vector<float> &vctPeoScore,  //检测到行人得分
		vector<CvRect> &vctPeoRect,  //检测到行人rect
		bool bUseGrayImgDet = true   //采用灰度图来检测HoG
		);

private:
	int   m_nHoGModelType;  //HoG模型类型

	int   m_nFeatDim; //特征维数
	int   m_nPeoStdW; //标准行人的宽度
	int	  m_nPeoStdH; //标准行人的高度

	int m_nMaxDetPeoWinCnt; //最多的检测到的行人窗口数目

	float *m_fAHoGSvmDetector;	   //HoG模型
	float *m_fAMultWinHogDesc;	   //多个窗口的HoG特征
	float *m_fAMultDetPeoHogDesc;  //多个检测出的行人的HoG特征
	float *m_fAOneWinHogDesc;      //单个窗口的HoG特征

	CvPoint *m_ptAWindowLtPt;      //窗口的左上点在给定图像中的位置
	CvRect  *m_rectAPeo;		   //检测到的行人rect
	float   *m_fAPeoScore;         //检测到的行人得分
	int     *m_nAWindowIdx;        //对应的窗口序号

	MvSimHOGDesc  m_hogDescriptor; //HoG检测类(自动构造)

	char  m_chSaveDir[104];   //存结果的目录
	bool  m_bSaveResult;	  //是否保存结果

	AnTypeDateStruct m_initDateTime;  //结构体启动的时间

	int m_nSaveSampleCnt;     //所保存的样本数目

}StruHoGPeoApp;


//------------HoG应用的封装结构体------------//
enum enumUseHoGSvmMode   //所采用的Ho的模式
{ 
	EASY_PEO_ALL_HOG_MODE = 1,
	HARD_PEO_ALL_HOG_MODE,
	EASY_PEO_HEAD_HOG_MODE,
	HARD_PEO_HEAD_HOG_MODE 
};

typedef struct StruBaseSimHoGPeoDetApp  //HoG应用结构体
{
public:
	StruBaseSimHoGPeoDetApp( )	{ mvInitVar( ); }

	//初始化SimHoG的应用
	void mvInitSimHoGApplication( );

	//释放
	void mvUnInitSimHoGApplication( );

	//在给定图上进行HoG检测
	bool mvDetWithSimHoG( 
			vector<float> &vctBestPeoScore,	 //检测到的最好行人得分
			vector<CvRect> &vctrectBestPeo,  //检测到的最好行人rect
			IplImage *pSrcRgbImg,			 //给定的彩色图像
			CvPoint ptLtSrc,				 //给定图上待检测区域左上点
			CvPoint ptRbSrc,				 //给定图上待检测区域右下点
			CvPoint ptPeoWhSrc,				 //给定图检测区上行人的标准宽高
			enumUseHoGSvmMode nUseHoGMode,   //所使用的HoG模型
			double dTsNow,					 //当前的时间戳(单位s,用于存图)
			int    nResCnt = 3,          //对行人模板缩放的次数--3
			float  fMaxResScale = 1.2f,  //最大缩放比率--1.2
			float  fResizeRate  = 1.2f,	 //每次的缩放的比率--1.2
			float  fPeoScoreTh  = 0.0f,  //行人判断的阈值--0.0
			int    nPeoCntTh = 1         //行人判断的个数阈值--1
		);
	
private:
	//初始化变量
	void mvInitVar( );

private:
	int  m_nModelCnt;

	bool m_bInitHogPeoApp[10];
	StruHoGPeoApp  *m_struHoGPeoApp;  //行人HoG

	bool m_bInitHogHeadApp[10];
	StruHoGPeoApp  *m_struHoGHeadApp; //人头HoG  

#ifdef SHOW_NEW_HOGDET_RESULT
	IplImage *m_pShowPeoDetImg;  //拷贝用于绘制检测结果的图像
	double m_dTsCopyDrawImg;     //m_pShowPeoDetImg更新的时间戳
#endif

}MvBaseSimHoGPeoDetApp;


typedef struct StruHoGPeoAreaChoseApp  //HoG行人选择应用的结构体
{
	//获取默认的行人权重值32S的图像
	static IplImage* mvGet32SWeightImg( int nH, int nW );

}MvHoGPeoAreaChoseApp;

#endif