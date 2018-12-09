#ifndef __CAM_SHIFT_DEMO_H
#define __CAM_SHIFT_DEMO_H

#include "libHeader.h"

#define MAX_HIST_CNT 100


//可循环使用的存储数组
typedef struct StruCycleSaveArray 
{
public:
	StruCycleSaveArray( ){ mvInitVar( ); };

	void mvInitVar( );

	//获取区域id在存储中的序号
	int mvFindSaveIdx( 
			bool &bFitstApp,      //是否为第一次出现
			int  nRectId,		   //所给的ID
			double dTsNow,		   //当前的时间戳-s
			double dClearTime=60.0 //需清理的时间-s
		);

private:
	int     m_nARectID[MAX_HIST_CNT]; 
	double  m_dAAppTs[MAX_HIST_CNT]; 

}MvCycleSaveArray;


class MvClassCamShift
{
public:
	MvClassCamShift( ) { mvInitVar( ); };

	~MvClassCamShift( ){ mvUninitCamShift( ); };

	//CamShift跟踪主接口
	void mvCamShift( 
			vector<CvRect> &trackResult, //OUT:跟踪得到的窗口
			vector<CvRect> &vRectS,		 //IN:选择的要跟踪的区域
			vector<int> &vRectId,		 //IN:选择的要跟踪的区域
			vector<bool> &vJustCalcHist, //IN:只计算要跟踪区域的直方图
			IplImage *image,         //IN:输入图像
			double   dTsNow,		 //IN:当前的时间戳-s	
			IplImage *pFkMask=NULL   //IN:前景mask图像区域
		);

	//灰度的CamShift跟踪主接口
	void mvGrayCamShift( 
			vector<CvRect> &vTrackRes,   //OUT:跟踪得到的窗口
			vector<CvRect> &vRectS,		 //IN:选择的要跟踪的区域
			vector<int> &vRectId,		 //IN:选择的要跟踪的区域
			vector<bool> &vJustCalcHist, //IN:只计算要跟踪区域的直方图
			IplImage *image,		 //IN:输入图像
			double   dTsNow,		 //IN:当前的时间戳-s
			IplImage *pFkMask=NULL   //IN:前景mask图像区域	
		);

	//释放
	void mvUninitCamShift( );

private:
	//初始化变量
	void mvInitVar( );

   //对给定区域进行CamShift跟踪
    bool mvRectCamShift( 
		   CvRect &trackResult,   //OUT:跟踪得到的窗口
		   CvRect &selection,     //IN:选择的要跟踪的区域
		   int    nRectId,        //IN:区域的ID
		   double dTsNow,         //IN:当前的时间戳
		   bool  bJustCalcHist    //IN:是否只计算当前块的hist
	   );

private:
	bool   m_bInit;     //是否完成了初始化
	bool   m_bUseMask;  //是否采用mask来进行
	CvRect m_rectTrack;
	
	MvCycleSaveArray  m_IdSaveArray;  //id存储器

private:
	IplImage *m_hsv, *m_hue;
	IplImage *m_pMask, *m_backproject;

	int      m_nARectID[MAX_HIST_CNT]; 
	CvHistogram *m_histA[MAX_HIST_CNT];
	CvHistogram *m_hist;

	CvBox2D m_track_box;
	CvConnectedComp m_track_comp;
};


#endif